#version 120

uniform sampler2D shadowMap;
uniform vec3 lightPos;
uniform float ka;

varying vec4 vColor;
varying vec3 vNormal;
varying vec3 vWorldPos;
varying vec4 vShadowCoord;

void main() {
    // shadow map lookup via perspective divide -> [0,1]
    vec3 sc = vShadowCoord.xyz / vShadowCoord.w;
    sc = sc * 0.5 + 0.5;

    // shadow test with slope-scaled bias
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vWorldPos);
    float cosTheta = clamp(dot(N, L), 0.05, 1.0);
    float bias = 0.005 / cosTheta;
    bias = min(bias, 0.02);

    float shadow = 1.0;
    if (sc.x >= 0.0 && sc.x <= 1.0 && sc.y >= 0.0 && sc.y <= 1.0) {
        float closestDepth = texture2D(shadowMap, sc.xy).r;
        if (sc.z - bias > closestDepth)
            shadow = 0.0;
    }

    // diffuse lighting
    float diff = max(dot(N, L), 0.0);
    vec3 lit = vColor.rgb * (ka + (1.0 - ka) * diff * shadow);

    gl_FragColor = vec4(lit, 1.0);
}
