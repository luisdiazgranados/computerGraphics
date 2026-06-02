//#define GEOM_SHADER

#include "CGInterface.h"

#ifndef NO_CG
#include "v3.h"
#include "scene.h"

#include <iostream>

using namespace std;

CGInterface::CGInterface() {};

void CGInterface::PerSessionInit() {

  glEnable(GL_DEPTH_TEST);

  CGprofile latestVertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
#ifdef GEOM_SHADER
  CGprofile latestGeometryProfile = cgGLGetLatestProfile(CG_GL_GEOMETRY);
#endif
  CGprofile latestPixelProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

#ifdef GEOM_SHADER
  if (latestGeometryProfile == CG_PROFILE_UNKNOWN) {
	  cerr << "ERROR: geometry profile is not available" << endl;
    exit(0);
  }

  cgGLSetOptimalOptions(latestGeometryProfile);
  CGerror Error = cgGetError();
  if (Error) {
	  cerr << "CG ERROR: " << cgGetErrorString(Error) << endl;
  }

  cout << "Info: Latest GP Profile Supported: " << cgGetProfileString(latestGeometryProfile) << endl;

  geometryCGprofile = latestGeometryProfile;
#endif

  cout << "Info: Latest VP Profile Supported: " << cgGetProfileString(latestVertexProfile) << endl;
  cout << "Info: Latest FP Profile Supported: " << cgGetProfileString(latestPixelProfile) << endl;

  vertexCGprofile = latestVertexProfile;
  pixelCGprofile = latestPixelProfile;
  cgContext = cgCreateContext();


}

bool ShaderOneInterface::PerSessionInit(CGInterface *cgi) {

	cerr << "INFO: entering ShaderOneInterface::PerSessionInit" << endl;
#ifdef GEOM_SHADER
  geometryProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->geometryCGprofile, "GeometryMain", NULL);
  if (geometryProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Geometry Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }
#endif

  vertexProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->vertexCGprofile, "VertexMain", NULL);
  if (vertexProgram == NULL) {
    CGerror Error = cgGetError();
    cerr << "Shader One Vertex Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

  fragmentProgram = cgCreateProgramFromFile(cgi->cgContext, CG_SOURCE, 
    "CG/shaderOne.cg", cgi->pixelCGprofile, "FragmentMain", NULL);
  if (fragmentProgram == NULL)  {
    CGerror Error = cgGetError();
    cerr << "Shader One Fragment Program COMPILE ERROR: " << cgGetErrorString(Error) << endl;
    cerr << cgGetLastListing(cgi->cgContext) << endl << endl;
    return false;
  }

	// load programs
#ifdef GEOM_SHADER
	cgGLLoadProgram(geometryProgram);
#endif
	cgGLLoadProgram(vertexProgram);
	cgGLLoadProgram(fragmentProgram);

	// build some parameters by name such that we can set them later...
  vertexModelViewProj = cgGetNamedParameter(vertexProgram, "modelViewProj" );
  vertexOCenter = cgGetNamedParameter(vertexProgram, "oCenter");
  vertexSRadius = cgGetNamedParameter(vertexProgram, "sRadius");
  vertexMFraction = cgGetNamedParameter(vertexProgram, "mFraction");
  fragmentEye = cgGetNamedParameter(fragmentProgram, "eye");
  fragmentV0 = cgGetNamedParameter(fragmentProgram, "V0");
  fragmentV1 = cgGetNamedParameter(fragmentProgram, "V1");
  fragmentV3 = cgGetNamedParameter(fragmentProgram, "V3");
#ifdef GEOM_SHADER
  geometryModelViewProj = cgGetNamedParameter(geometryProgram, "modelViewProj" );
#endif
  return true;

}

void ShaderOneInterface::PerFrameInit() {

	//set parameters
	cgGLSetStateMatrixParameter(vertexModelViewProj, 
		CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
	V3 oCenter = scene->tms[0].GetCenter();
//	oCenter[0] += 0.5f;
//	oCenter[1] += 0.5f;
//	oCenter[2] += 150.0f + 0.5f;
//	cerr << endl << "INFO: ojbect center: " << oCenter << endl;
//	oCenter = V3(1.0f, 0.0f, 1.0f);
	cgGLSetParameter3fv(vertexOCenter, (const float*)&oCenter);
	float sRadius = 35.0f;
	cgGLSetParameter1f(vertexSRadius, sRadius);
	cgGLSetParameter1f(vertexMFraction, scene->mFraction);
	cgGLSetParameter3fv(fragmentEye, (const float*)&(scene->ppc->C + V3(0.11f, 0.12f, 0.13f)));
	cgGLSetParameter3fv(fragmentV0, (const float*)&(scene->tms[1].verts[0]));
	cgGLSetParameter3fv(fragmentV1, (const float*)&(scene->tms[1].verts[1]));
	cgGLSetParameter3fv(fragmentV3, (const float*)&(scene->tms[1].verts[3]));

#ifdef GEOM_SHADER
  cgGLSetStateMatrixParameter(
    geometryModelViewProj, 
	  CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);
#endif

}

void ShaderOneInterface::PerFrameDisable() {
}


void ShaderOneInterface::BindPrograms() {

#ifdef GEOM_SHADER
  cgGLBindProgram(geometryProgram);
#endif
  cgGLBindProgram(vertexProgram);
  cgGLBindProgram(fragmentProgram);

}

void CGInterface::DisableProfiles() {

  cgGLDisableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLDisableProfile(geometryCGprofile);
#endif
  cgGLDisableProfile(pixelCGprofile);

}

void CGInterface::EnableProfiles() {

  cgGLEnableProfile(vertexCGprofile);
#ifdef GEOM_SHADER
  cgGLEnableProfile(geometryCGprofile);
#endif
  cgGLEnableProfile(pixelCGprofile);

}

#endif // NO_CG

