#version 460 core
out vec4 FragColor;

uniform sampler2D shadowMap;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

//uniform sampler2D diffuseTexture;

uniform vec3 lightPos;
uniform vec3 viewPos;

#define NR_POINT_LIGHTS 4

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;

uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

uniform bool useDiffuse;
uniform bool useAmbient;
uniform bool useSpecular;
uniform bool useBlinn;

uniform bool useFlashlight;
uniform bool useDirectionalLight;
uniform bool usePointLight;
uniform bool useShadows;

//depth testing
uniform bool showDepthBuffer;
float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

//light intensity
uniform float flashlightIntensity;
uniform float directionalLightIntensity;
uniform float pointLightIntensity;

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace);

void main() {    
    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 result = vec3(0.0);
    

    // directional lighting
    if(useDirectionalLight) {
        result = CalcDirLight(dirLight, norm, viewDir) * directionalLightIntensity;
    }
    // point lights
    if(usePointLight) {
        for(int i = 0; i < NR_POINT_LIGHTS; i++)
            result += CalcPointLight(pointLights[i], norm, FragPos, viewDir) * pointLightIntensity; 
    }   
    // spot light
    if(useFlashlight) {
        result += CalcSpotLight(spotLight, norm, FragPos, viewDir) * flashlightIntensity;
    }
    
    if (!showDepthBuffer) {
        //FragColor = vec4(result, 1.0);
        //float shadow = ShadowCalculation(fs_in.FragPosLightSpace);                      
       // vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
        FragColor = vec4(result, 1.0);
    } else {
        float depth = LinearizeDepth(gl_FragCoord.z) / far;
        FragColor = vec4(vec3(depth), 1.0);
    }
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.002 * (1.0 - dot(normal, lightDir)), 0.0005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0;
    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    if(useShadows) {
        //return ambient + (1.0 - ShadowCalculation(FragPosLightSpace)) * (diffuse + specular);
        return (texture(material.diffuse, TexCoords).rgb * (diffuse * (1.0 - ShadowCalculation(FragPosLightSpace)) + ambient) + texture(material.specular, TexCoords).r * specular  * (1.0 - ShadowCalculation(FragPosLightSpace))) * (dirLight.diffuse + dirLight.ambient + dirLight.specular);
    } else {
        return (ambient + diffuse + specular);
    }
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0;
    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0;
    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    return (ambient + diffuse + specular);
}