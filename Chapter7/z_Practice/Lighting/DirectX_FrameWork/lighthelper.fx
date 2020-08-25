struct DirectionalLight //태양광
{
	float4 Ambient; //주변광
	float4 Diffuse; //난반사광 
	float4 Specular; //정반사광
	float3 Direction; //빛의 방향
	float pad; //4차원벡터단위로 채우기위해 필요한것
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att; //거리에 따라 감소하는 빛의 세기를 조정
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float4 Reflect;
};

void ComputeDirectionalLight(Material mat, DirectionalLight L,
	float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -L.Direction;
	lightVec = normalize(lightVec);

	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	{
		if (diffuseFactor > 0.0f)
		{
			float3 v = reflect(-lightVec, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
			diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
			spec = specFactor * mat.Specular * L.Specular;
		}
	}
}

void ComputePointLight(Material mat, PointLight L,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = L.Position - pos;

	//표면점과 광원사이의 거리
	float d = length(lightVec);
	
	if (d > L.Range)
		return;

	normalize(lightVec);

	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	{
		if (diffuseFactor > 0.0f)
		{
			float3 v = reflect(-lightVec, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
			diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
			spec = specFactor * mat.Specular * L.Specular;
		}

		float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

		diffuse *= att;
		spec *= att;
	}
}

void ComputeSpotLight(Material mat, SpotLight L,
	float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = L.Position - pos;

	//표면점과 광원사이의 거리
	float d = length(lightVec);

	if (d > L.Range)
		return;

	normalize(lightVec);

	ambient = mat.Ambient * L.Ambient;

	float diffuseFactor = dot(lightVec, normal);

	[flatten]
	{
		if (diffuseFactor > 0.0f)
		{
			float3 v = reflect(-lightVec, normal);
			float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);
			diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
			spec = specFactor * mat.Specular * L.Specular;
		}

		float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);
		float att = spot / dot(L.Att, float3(1.0f, d, d * d));

		ambient *= att;
		diffuse *= att;
		spec *= att;
	}
}