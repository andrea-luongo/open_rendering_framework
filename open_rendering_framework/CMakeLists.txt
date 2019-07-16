
#
#  Copyright (c) 2008 - 2009 NVIDIA Corporation.  All rights reserved.
#
#  NVIDIA Corporation and its licensors retain all intellectual property and proprietary
#  rights in and to this software, related documentation and any modifications thereto.
#  Any use, reproduction, disclosure or distribution of this software and related
#  documentation without an express license agreement from NVIDIA Corporation is strictly
#  prohibited.
#
#  TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED *AS IS*
#  AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS OR IMPLIED,
#  INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
#  PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS BE LIABLE FOR ANY
#  SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER (INCLUDING, WITHOUT
#  LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF
#  BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) ARISING OUT OF THE USE OF OR
#  INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS BEEN ADVISED OF THE POSSIBILITY OF
#  SUCH DAMAGES
#




set(sources
	AnisotropicMaterial.cpp
	Background.cpp
	BackgroundTabGui.cpp
	Camera.cpp
	CameraTabGui.cpp
	DiffuseMaterial.cpp
	FlatMaterial.cpp
	Geometry.cpp
	GeometryTabGui.cpp
	GuiWindow.cpp
	HDRLoader.cpp
	ImageLoader.cpp
	Integrator.cpp
	IntegratorTabGui.cpp
	LambertianInterfaceMaterial.cpp
	Light.cpp
	LightTabGui.cpp
	MetallicMaterial.cpp
	NormalMaterial.cpp
	ObjLoader.cpp
	OptixScene.cpp
	OptixSceneLoader.cpp
	OptixWindow.cpp
	PPMLoader.cpp
	RoughDiffuseMaterial.cpp
	RoughTranslucentMaterial.cpp
	RoughTransparentMaterial.cpp
	ScatteringMaterial.cpp
	TranslucentMaterial.cpp
	TransparentMaterial.cpp
	glm.cpp
	main.cpp
	
	${CMAKE_CURRENT_BINARY_DIR}/../sampleConfig.h
	Background.h
	BackgroundTabGui.h
	Camera.h
	CameraTabGui.h
	Envmap.h
	Fresnel.h
	Geometry.h
	GeometryTabGui.h
	GuiWindow.h
	HDRLoader.h
	ImageLoader.h
	Integrator.h
	IntegratorTabGui.h
	Light.h
	LightSampler.h
	LightTabGui.h
	Material.h
	md5.h
	Microfacet.h
	MicrofacetBeckmann.h
	MicrofacetGGX.h
	MyComplex.h
	ObjLoader.h
	OptixScene.h
	OptixSceneLoader.h
	OptixWindow.h   
	PPMLoader.h
	ScatteringMaterial.h
	dipoles/directional_dipole.h
	glm.h
	helpers.h
	random.h
	sampler.h
	Texture.h
	structs.h
	AnisotropicStructures.h
    dipoles/rough_directional_dipole.h
    dipoles/rough_standard_dipole.h
    dipoles/standard_dipole.h
	
	CUDA_files/arealight_shader.cu
	CUDA_files/constantbg.cu
	CUDA_files/diffuse_shader.cu
	CUDA_files/envmap_background.cu
	CUDA_files/flat_shader.cu
	CUDA_files/lambertian_interface_shader.cu
	CUDA_files/metallic_shader.cu
	CUDA_files/normal_shader.cu
	CUDA_files/obj_material.cu
	CUDA_files/path_tracer.cu
	CUDA_files/rough_transparent_shader.cu
	CUDA_files/sample_camera.cu
	CUDA_files/subsurface_scattering_shader.cu
	CUDA_files/rough_subsurface_scattering_shader.cu
	CUDA_files/transparent_shader.cu	
	CUDA_files/triangle_mesh.cu
	CUDA_files/triangle_mesh_small.cu
	CUDA_files/disk_mesh.cu
	CUDA_files/rough_diffuse_shader.cu
	CUDA_files/sphere_mesh.cu
	CUDA_files/depth_tracer.cu
	CUDA_files/depth_shader.cu
	CUDA_files/rough_depth_shader.cu
	CUDA_files/anisotropic_structure_shader.cu
  )
  


if(GLUT_FOUND AND OPENGL_FOUND)
  include_directories(${GLUT_INCLUDE_DIR})
  add_definitions(-DGLUT_FOUND -DGLUT_NO_LIB_PRAGMA)
  add_definitions(-DNOMINMAX)
  # See top level CMakeLists.txt file for documentation of OPTIX_add_sample_executable.
  OPTIX_add_sample_executable( qt_optix_framework
	${sources}
    )
	
else()
  # GLUT or OpenGL not found
  message("Disabling render, which requires glut and opengl.")
endif()


