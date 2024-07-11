#version 330 core
#define R iResolution.xy
#define PI 3.14159265

#define center R*0.5
#define L 1.
#define P 50.

#define A 20.
#define light_dir normalize(vec3(0.36,0.6,0.7))

out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform float time;
uniform vec2 resolution;

uniform sampler2D texture1;
uniform sampler2D textureBg;

uniform float rotation;
uniform float speed;
uniform float frequency;

float hash12(vec2 p)
{
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float perlin(vec2 p, out vec2 grad)
{
   vec2 pi = floor(p);
   vec2 pf = p - pi;
   vec2 pfc = 0.5 - 0.5*cos(pf*PI);
   vec2 pfs = sin(pf*PI);
   
   const vec2 a = vec2(0.,1.);
   
   float a00 = hash12(pi+a.xx);
   float a01 = hash12(pi+a.xy);
   float a10 = hash12(pi+a.yx);
   float a11 = hash12(pi+a.yy);
   
   float x1 = mix(a00, a01, pfc.y);
   float x2 = mix(a10, a11, pfc.y);
   float y1 = mix(a00, a10, pfc.x);
   float y2 = mix(a01, a11, pfc.x);
    
   grad = vec2((x2 - x1)*pfs.x, (y2 - y1)*pfs.y);
   
   return 0.5 + 0.5*mix(x1, x2, pfc.x);
}

vec4 dir(vec2 p, vec2 freq, float phase)
{
    float x = dot(p, freq) + phase;
    return vec4(cos(x), sin(x), -freq.x*sin(x), freq.y*cos(x));
}

vec4 wind(vec2 p, float t)
{
    t*=5.;
    
    vec4 dx = 0.04*dir(p, vec2(-0.3, -0.5), 1.133*t)+
              0.1*dir(p, vec2(0.1, 0.11), 0.431*t)+
              0.2*dir(p, vec2(-0.12, 0.1), 0.256*t);
    return 0.6*dx;
}

vec3 clothl(vec2 p, out vec2 GRAD)
{
    vec3 f = vec3(0.);
    vec3 d = vec3(0., 0., 1.);
    GRAD = vec2(0.);
    for(int i = 0; i < 2; i++)
    {
        vec2 grad;
        f += perlin(d.z*p + d.xy + vec2(i)*1e3, grad)*vec3(1.);
        GRAD += d.z*grad;
        d = vec3(d.x,d.y,0.3)*d + vec3(2.64, 1.5446, 0.);
    }
    return f;
}

#define cell_s 0.2
vec3 cloth_pattern(vec2 pos, out vec3 normal)
{
    //create the weaving pattern
    vec2 g1, g2;
    const vec2 strand = vec2(0.02, 1.5); 
 	vec3 a = vec3(1.,.7,0.)*clothl(strand*pos, g1);
    vec3 b = 1.2*vec3(1.,0.2,0.25)*clothl(strand.yx*pos, g2);
    //checker modulation
    float M = (2.*smoothstep(-0.5,0.5,sin(PI*pos.x*cell_s)) - 1.)*
        	  (2.*smoothstep(-0.5,0.5,sin(PI*pos.y*cell_s)) - 1.);
    float Ma = smoothstep(-0.2,0.2,M);
    normal = normalize(vec3(2.*mix(g1*strand, g2*strand.yx, Ma),1.));
    return 3.*mix(a,b,Ma);
}

vec4 cloth(vec2 p)
{
    vec4 col = vec4(0.);
    vec4 dx = wind(p*0.1, time);
    //displacement normal
    vec3 normal = normalize(vec3(-A*dx.zw, 1.));
  	//diceplacement
    vec2 pos = p + A*dx.xy; 
    
    // //texture
    // vec3 c, N;
    // cloth_pattern(pos, N);

    vec4 c = texture(texture1, 2.0 * pos / resolution);
    
    //cloth normal modulation
    // normal = normalize(1.6*normal + N);
    normal = normalize(1.6*normal);
    
    // magic shading stuff
	float d = clamp(dot(normal, light_dir), 0., 1.);
    float s = pow(d, 20.);
    float B = (0.08*d + 0.05 + 0.2*s) / (0.15); //brightness
    // col = vec4(B*c,1.); //blend
    col = B*c; //blend
   
    return col;
}

void main()
{
    vec4 texColor1 = tanh(2.*cloth(0.5*TexCoord*resolution));

    vec4 texColorBg = texture(textureBg, TexCoord);

    // vec4 blendedColor = 1.0 - (1.0 - texColor1) * (1.0 - texColorBg);
    vec4 blendedColor = texColor1.a > 2.0 ? texColor1 : texColorBg;

    FragColor = texColor1;
}