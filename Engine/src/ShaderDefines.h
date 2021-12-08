#pragma once

/*
 * Start Slots For Shader Resources.
 * CB = constant buffers
 * S = sampler
 * T = textures
 * SB = structured buffer
 * RW = read/write buffer
 */	
											// old
#define CB_WORLD						0	// 0
#define CB_CAMERA						1	// 1
#define CB_MATERIAL_CONSTANTS			2	// 2
#define CB_MATERIAL_PROPERTIES			3	// 3
#define CB_LIGHTS_INFO					4	// 4
#define CB_IS_COLLIDING					5	// 5
//#define CB_SCREEN_TO_VIEW_PARAMS		6	// 6
#define CB_DISPATCH_PARAMS				6	// 7
#define CB_PARTICLE_UPDATE				7	// 8
//#define CB_PARTICLE_MODE				9	// 9
#define CB_DECAL_INFO					8	// 10
//#define CB_DELTA						11	// 6
#define CB_BLUR_SETTINGS				9	// 11
#define CB_TEXTURE_EFFECTS				10	// 7
#define CB_DOF_SETTINGS					11	// 12
#define CB_SKYBOX_TINT					12	// 13

#define S_POINT 						0	
#define S_LINEAR						1
#define S_ANISOTROPIC 					2
#define S_CUBEMAP						3
										
#define T_DEPTH							0
#define T_ALBEDO						1
#define T_NORMAL						2
#define T_METALNESS						3
#define T_ROUGHNESS						4
#define T_AOMAP							5
#define T_DISPLACE						6
#define T_OPACITYMASK					7
#define T_POINT_LIGHT_GRIP				8
#define T_DECAL							9	// 12
#define T_SHADOW_MAPS					10	// 13

#define SB_BONE_TRANSFORMS				11	// 9	 
#define SB_LIGHTS						12	// 10 	
#define SB_FRUSTUMS_IN					13		
#define SB_LIGHT_INDEX_LIST				14		
#define SB_DECALDATA					15	// 16  	
#define SB_PARTICLES_SRV				16	// 17	
#define SB_RANDOM_NUMBERS				17	// 18
												
#define T_UNDERWATER_EDGE				18	// 18	
#define T_WATER_FLOOR_TEXTURE			19	// 19	
#define T_WATER_TEXTURE					20	// 20
#define T_WATER_TEXTURE_N				21	// 21
#define T_WATER_BLEND					22	// 22	
#define T_RADIANCE						23	// 96	
#define T_IRRADIANCE					24	// 97	
#define T_SKY							25	// 98	
#define T_BRDFLUT						26	// 99	

#define RW_BUFFER_READ					0		
#define RW_BUFFER_OUT					1		
#define RW_IN_FOCUS						2		
#define RW_OUT_OF_FOCUS					3		
#define RW_DOF_OUT						4		
#define RW_WATER_FLOOR_TEXTURE			5		
#define RW_WATER_TEXTURE				6		
#define RW_WATER_TEXTURE_N				7		
#define RW_UNDERWATER_EDGE				8 // 9		
#define RW_PARTICLES_UAV				9 // 7		
#define RW_FRUSTUMS_OUT					10		
#define RW_OPAQ_LIGHT_INDEX_COUNTER		11		
#define RW_TRANS_LIGHTS_INDEX_COUNTER	12		
#define RW_OPAQ_LIGHT_INDEX_LIST		13		
#define RW_TRANS_LIGHT_INDEX_LIST		14		
