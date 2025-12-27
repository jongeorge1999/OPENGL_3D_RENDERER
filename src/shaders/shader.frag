#version 460 core
out vec4 FragColor;

uniform sampler2D shadowMap;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D normal;
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

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float far_plane;

#define MAX_POINT_LIGHTS 2
//#define NR_POINT_LIGHTS 4
uniform int NR_POINT_LIGHTS;

uniform int TEST;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec4 FragPosLightSpace;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in mat3 TBN;

uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

uniform vec3 pointLightPos[MAX_POINT_LIGHTS];
uniform samplerCube depthCubeMap[MAX_POINT_LIGHTS];

uniform bool useDiffuse;
uniform bool useAmbient;
uniform bool useSpecular;
uniform bool useBlinn;

uniform bool useFlashlight;
uniform bool useDirectionalLight;
uniform bool usePointLight;
uniform bool useShadows;

uniform bool useNormalMaps;

uniform float shadowFactor;

uniform bool useSmoothShadows;

uniform float exposure;

uniform float shadowBias;
uniform float dirShadowBias;

uniform float pointLightRadius;

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

// array of offset direction for sampling
vec3 gridSamplingDisk[20] = vec3[]
(
   vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1), 
   vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
   vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
   vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
   vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

// function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int index);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal);
float PointShadowCalculation(vec3 fragPos, int index, vec3 normal);

void main() {

    const float gamma = 2.2;
    vec3 hdrColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    // properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 viewDirNormal = normalize(TangentViewPos - TangentFragPos); // normal mapping
    vec3 result = vec3(0.0);
    
    // for normal mapping
    vec3 normalMap = texture(texture_normal1, TexCoords).rgb;
    normalMap = normalMap * 2.0 - 1.0;

    // directional lighting
    if(useDirectionalLight) {
        if(useNormalMaps) { result = CalcDirLight(dirLight, normalMap, viewDirNormal) * directionalLightIntensity; } 
        else { result = CalcDirLight(dirLight, norm, viewDir) * directionalLightIntensity; }
    }
    
    // point lights
    if(usePointLight) {
        int n = min(NR_POINT_LIGHTS, MAX_POINT_LIGHTS);
        for(int i = 0; i < n; i++) { 
            if(useNormalMaps) {
                //result += CalcPointLight(pointLights[i], normalMap, FragPos, viewDirNormal, i) * pointLightIntensity; 
                result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, i) * pointLightIntensity; 
            } else {
                result += CalcPointLight(pointLights[i], norm, FragPos, viewDir, i) * pointLightIntensity; 
            }
        }
    }   
    // spot light
    if(useFlashlight) { result += CalcSpotLight(spotLight, norm, FragPos, viewDir) * flashlightIntensity; }
    
    if (!showDepthBuffer) {
        FragColor = vec4(result * mapped, 1.0);
    } else {
        float depth = LinearizeDepth(gl_FragCoord.z) / far;
        FragColor = vec4(vec3(depth), 1.0);
    }
}

float ShadowCalculation(vec4 fragPosLightSpace, vec3 normal) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;
    normal = normalize(Normal);
    //vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    vec3 lightDir = normalize(lightPos - FragPos);
    float bias = max(0.0005 * (1.0 - dot(normal, lightDir)), dirShadowBias);
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    if(projCoords.z > 1.0) { shadow = 0.0; }
    return shadow;
}

float PointShadowCalculation(vec3 fragPos, int index, vec3 normal)
{
    float shadow = 0.0;
    if(useSmoothShadows) {
        // get vector between fragment position and light position
        vec3 fragToLight = fragPos - pointLightPos[index];
        //vec3 fragToLight = TBN * fragPos - TBN * pointLightPos[index];
        float currentDepth = length(fragToLight);
        float bias = shadowBias;
        int samples = 20;
        //float viewDistance = length(TBN * viewPos - TBN * fragPos);
        float viewDistance = length(viewPos - fragPos);
        float diskRadius = (1.0 + (viewDistance / far_plane)) / pointLightRadius;
        for(int i = 0; i < samples; ++i) {
            float closestDepth = texture(depthCubeMap[index], fragToLight + gridSamplingDisk[i] * diskRadius).r;
            closestDepth *= far_plane;
            if(currentDepth - bias > closestDepth)
                shadow += 1.0;
        }
        shadow /= float(samples);
    } else {
        // get vector between fragment position and light position
        vec3 fragToLight = fragPos - pointLightPos[index];
        // use the light to fragment vector to sample from the depth map    
        float closestDepth = texture(depthCubeMap[index], fragToLight).r;
        // it is currently in linear range between [0,1]. Re-transform back to original value
        closestDepth *= far_plane;
        // now get current linear depth as the length between the fragment and light position
        float currentDepth = length(fragToLight);
        // now test for shadows
        vec3 lightDir = normalize(pointLightPos[index] - fragPos);
        // float bias = shadowBias; 
        float bias = max(shadowBias * (1.0 - dot(normal, lightDir)), shadowBias);
        shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    }
    return shadow;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = vec3(0.0);
    if(useNormalMaps) {
        lightDir = normalize(TangentLightPos - TangentFragPos); // normal mapping
    } else {
        lightDir = normalize(-light.direction);
    }
    // diffuse shading
    float diff = max(dot(lightDir, normal), 0.0);
    // specular shading
    float spec = 0.0;
    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    if (diff == 0.0) spec = 0.0;
    // combine results
    vec3 ambient = light.ambient * texture(texture_diffuse1, TexCoords).rgb;
    vec3 diffuse = light.diffuse * diff * texture(texture_diffuse1, TexCoords).rgb;
    vec3 specular = light.specular * spec * texture(texture_specular1, TexCoords).rgb;
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    if(useShadows) {
        return ambient + (1.0 - ShadowCalculation(FragPosLightSpace, normal)) * (diffuse + specular);
        //return (ambient + diffuse + specular);
        //return (texture(material.diffuse, TexCoords).rgb * (diffuse * (1.0 - ShadowCalculation(FragPosLightSpace)) + ambient) + texture(material.specular, TexCoords).r * specular  * (1.0 - ShadowCalculation(FragPosLightSpace))) * (dirLight.diffuse + dirLight.ambient + dirLight.specular);
    } else {
        return (ambient + diffuse + specular);
    }
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, int index) {
    vec3 lightDir = vec3(0.0);
    //if(useNormalMaps) {
        //lightDir = normalize(TBN * light.position - TangentFragPos); // normal mapping
    // } else {
         lightDir = normalize(light.position - fragPos);
    // }
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    float spec = 0.0;
    if (useBlinn) {
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
    } else {
        vec3 reflectDir = reflect(-lightDir, normal);
        spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    }
    if (diff == 0.0) spec = 0.0;
    // attenuation
    float distance = length(light.position - fragPos);
    //float distance = length(TBN * light.position - TangentFragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // combine results
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    if(useShadows) {
        //return ambient + (1.0 - ShadowCalculation(FragPosLightSpace)) * (diffuse + specular);
        //return (texture(material.diffuse, TexCoords).rgb * (diffuse * (1.0 - PointShadowCalculation(FragPos)) + ambient) + texture(material.specular, TexCoords).r * specular  * (1.0 - PointShadowCalculation(FragPos))) * (light.diffuse + light.ambient + light.specular);
        //return ambient + (1.0 - PointShadowCalculation(FragPos, index, normal)) * (diffuse + specular);
        //return (ambient + diffuse + specular);
        // Calculate the shadow factor
        float shadowFactorInt = 1.0 - PointShadowCalculation(FragPos, index, normal);

        // Mix shadowed and unshadowed contributions
        vec3 unshadowedColor = ambient + diffuse + specular;
        vec3 shadowedColor = ambient + unshadowedColor * shadowFactor; // Darken the unshadowed color

        // Interpolate based on shadow factor
        vec3 finalColor = mix(shadowedColor, unshadowedColor, shadowFactorInt);
        return finalColor;
    } else {
        return (ambient + diffuse + specular);
    }
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
    vec3 ambient = light.ambient * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    //on-off
    diffuse *= useDiffuse ? 1 : 0;
    specular *= useSpecular ? 1 : 0;
    ambient *= useAmbient ? 1 : 0;
    return (ambient + diffuse + specular);
}