#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D texture_diffuse1;
};

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace;
} fs_in;

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic; 
};
uniform DirectionalLight dLight;
uniform PointLight pLight;
uniform Material u_mat;
uniform vec3 u_viewPos;
uniform sampler2D depthMap;

vec3 color;

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir);
float ShadowCalculation(vec4 fragPosLightSpace);

void main()
{
   vec3 normal = normalize(fs_in.Normal);
   vec3 viewDir = normalize(u_viewPos - fs_in.FragPos);
   color = texture(u_mat.texture_diffuse1, fs_in.TexCoords).rgb;

   vec3 result = CalcDirLight(dLight, normal, viewDir);
   //result += CalcPointLight(pLight, normal, fs_in.FragPos, viewDir);
   FragColor = vec4(result, 1.0);
}

vec3 CalcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse
    float diff = max(dot(lightDir, normal), 0.0);
    // specular
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
    // combine
    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec;

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;

    return lighting;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    // specular
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), 32.0);

    vec3 ambient = light.ambient * color;
    vec3 diffuse = light.diffuse * diff * color;
    vec3 specular = light.specular * spec;

    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1 / (light.constant + (light.linear * distance) + (light.quadratic * distance * distance));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

float ShadowCalculation(vec4 fragPosLightSpace) {
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // transform to range [0, 1] to use to sample from depth map
    // closest depth from light's POV
    float closestDepth = texture(depthMap, projCoords.xy).r;
    float currentDepth = projCoords.z; // the depth of the current fragment the fragment shader is running on
    // depth comparison
    float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
    return shadow;
}
