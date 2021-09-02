Shader "Vts/UnlitTransparent"
{
	SubShader
	{
		Tags
		{
			"Queue" = "Opaque"
			"RenderType" = "Transparent"
			"ForceNoShadowCasting" = "True"
			"IgnoreProjector" = "True"
		}

		Pass
		{
			Tags { "LightMode" = "ForwardBase" }

			Blend SrcAlpha OneMinusSrcAlpha
			ZWrite Off
			Cull Off
			Offset 0, -1000

			CGPROGRAM
			#pragma vertex vert
			#pragma fragment frag
			#pragma target 3.5
			#pragma multi_compile __ VTS_ATMOSPHERE
			#include "VtsUnlit.cginc"
			ENDCG
		}
	}
}
