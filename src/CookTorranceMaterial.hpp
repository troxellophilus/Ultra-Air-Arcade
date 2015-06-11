class CookTorranceMaterial
{
public:
	// Cook torrance values
    float roughness; // 0 : smooth, 1: rough
    float fresnel; // fresnel reflectance at normal incidence
    float geometric; // fraction of diffuse reflection (specular reflection = 1 - k)

    CookTorranceMaterial(float roughness, float fresnel, float geometric):
    roughness(roughness),
    fresnel(fresnel),
    geometric(geometric)
    {

    }
};