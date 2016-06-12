#version 330 core

layout (std140) uniform Material {
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    vec4 emissive;
    float shininess;
    int texCount;
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

#define MAX_DIR_LIGHTS 10
#define MAX_POINT_LIGHTS 10
#define MAX_SPOT_LIGHTS 10

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 OutColor;

uniform vec3 viewPos;

uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];
uniform SpotLight flashLight;
uniform int numOfDirLights = 0;
uniform int numOfPointLights = 0;
uniform int numOfSpotLights = 0;
uniform bool flashLightOn;

uniform sampler2D texUnit;
uniform bool lighting;
uniform bool isTextured;
uniform bool forceTextured;

// Function prototypes
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

void SetLightColor(vec3 lambient, vec3 ldiffuse, vec3 lspecular, inout vec4 _ambient, inout vec4 _diffuse, inout vec4 _specular, float diff, float spec);

void main()
{
    // Properties
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    vec4 result;
    
    if (lighting) {
        // == ======================================
        // Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
        // For each phase, a calculate function is defined that calculates the corresponding color
        // per lamp. In the main() function we take all the calculated colors and sum them up for
        // this fragment's final color.
        // == ======================================
        // Phase 1: Directional lighting
        if (numOfDirLights > 0 && numOfDirLights <= MAX_DIR_LIGHTS) {
            for (int i = 0; i < numOfDirLights; ++i) {
                result += CalcDirLight(dirLights[i], norm, viewDir);
            }
        }
        
        // Phase 2: Point lights
        if (numOfPointLights > 0 && numOfPointLights <= MAX_POINT_LIGHTS) {
            for (int i = 0; i < numOfPointLights; ++i) {
                result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);  
            }
        }
        
        // Phase 3: Spot light
        if (numOfSpotLights > 0 && numOfSpotLights <= MAX_SPOT_LIGHTS) {
            for (int i = 0; i < numOfSpotLights; ++i) {
                result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);
            }
        }
 
        if (flashLightOn) {
            result += CalcSpotLight(flashLight, norm, FragPos, viewDir);
        }
    } else {
        if (isTextured) {
            if (forceTextured) {
                result = texture(texUnit, TexCoords);
            } else {
                if (texCount == 0) {
                    result = diffuse;
                } else {
                    result = texture(texUnit, TexCoords);
                }
            }
        } else {
            result = diffuse;
        }
    }

    result += vec4(emissive.xyz, 0.0f);
    
    OutColor = result;
}

// Calculates the color when using a directional light.
vec4 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Combine results
    vec4 _ambient;
    vec4 _diffuse;
    vec4 _specular;
    
    SetLightColor(light.ambient, light.diffuse, light.specular, _ambient, _diffuse, _specular, diff, spec);

    return (_ambient + _diffuse + _specular);
}

// Calculates the color when using a point light.
vec4 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Combine results
    vec4 _ambient;
    vec4 _diffuse;
    vec4 _specular;
    
    SetLightColor(light.ambient, light.diffuse, light.specular, _ambient, _diffuse, _specular, diff, spec);

    _ambient *= vec4(vec3(attenuation), 0.0f);
    _diffuse *= vec4(vec3(attenuation), 0.0f);
    _specular *= vec4(vec3(attenuation), 0.0f);
    return (_ambient + _diffuse + _specular);
}

// Calculates the color when using a spot light.
vec4 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // Diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // Specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    // Spotlight intensity
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // Combine results
    vec4 _ambient;
    vec4 _diffuse;
    vec4 _specular;

    SetLightColor(light.ambient, light.diffuse, light.specular, _ambient, _diffuse, _specular, diff, spec);

    _ambient *= vec4(vec3(attenuation), 0.0f) * vec4(vec3(intensity), 0.0f);
    _diffuse *= vec4(vec3(attenuation), 0.0f) * vec4(vec3(intensity), 0.0f);
    _specular *= vec4(vec3(attenuation), 0.0f) * vec4(vec3(intensity), 0.0f);
    return (_ambient + _diffuse + _specular);
}

void SetLightColor(vec3 lambient, vec3 ldiffuse, vec3 lspecular, inout vec4 _ambient, inout vec4 _diffuse, inout vec4 _specular, float diff, float spec)
{
    if (isTextured) {
        if (forceTextured) {
            _ambient = vec4(lambient, 1.0f) * texture(texUnit, TexCoords);
            _diffuse = vec4(ldiffuse, 1.0f) * vec4(vec3(diff), 0.0f) * texture(texUnit, TexCoords);
            _specular = vec4(lspecular, 1.0f) * vec4(vec3(spec), 0.0f) * texture(texUnit, TexCoords);
        } else {
            if (texCount == 0) {
                _ambient = vec4(lambient, 1.0f) * diffuse;
                _diffuse = vec4(ldiffuse, 1.0f) * vec4(vec3(diff), 0.0f) * diffuse;
                _specular = vec4(lspecular, 1.0f) * vec4(vec3(spec), 0.0f) * specular;
            } else {
            _ambient = vec4(lambient, 1.0f) * texture(texUnit, TexCoords);
            _diffuse = vec4(ldiffuse, 1.0f) * vec4(vec3(diff), 0.0f) * texture(texUnit, TexCoords);
            _specular = vec4(lspecular, 1.0f) * vec4(vec3(spec), 0.0f) * texture(texUnit, TexCoords);
            }
        }
    } else {
        _ambient = vec4(lambient, 1.0f) * diffuse;
        _diffuse = vec4(ldiffuse, 1.0f) * vec4(vec3(diff), 0.0f) * diffuse;
        _specular = vec4(lspecular, 1.0f) * vec4(vec3(spec), 0.0f) * specular;
    }
}