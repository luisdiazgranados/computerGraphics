#version 120

uniform sampler2D projTexture;
uniform vec3 projectorPos;
uniform float ka;

varying vec4 vColor;
varying vec3 vNormal;
varying vec3 vWorldPos;
varying vec4 vProjTexCoord;

void main() {
    vec3 N = normalize(vNormal);

    // projective texture lookup
    vec3 tc = vProjTexCoord.xyz / vProjTexCoord.w;
    tc = tc * 0.5 + 0.5;

    // check if fragment is in projector's frustum and facing projector
    vec3 projDir = normalize(projectorPos - vWorldPos);
    float facing = dot(N, projDir);

    vec3 finalColor;
    if (tc.x >= 0.0 && tc.x <= 1.0 && tc.y >= 0.0 && tc.y <= 1.0 &&
        tc.z > 0.0 && facing > 0.0) {
        vec3 texColor = texture2D(projTexture, tc.xy).rgb;
        // blend projected texture with surface color
        finalColor = mix(vColor.rgb, texColor, 0.7);
        // add basic diffuse
        float diff = max(dot(N, projDir), 0.0);
        finalColor = finalColor * (ka + (1.0 - ka) * diff);
    } else {
        // ambient only outside projection
        finalColor = vColor.rgb * ka;
    }

    gl_FragColor = vec4(finalColor, 1.0);
}
