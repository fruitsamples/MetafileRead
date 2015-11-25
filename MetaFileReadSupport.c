// QuickDraw 3d Sample Code//// This file contains utility routines for QuickDraw 3d sample code.// Shows how to read a metafile and render it.//// Created 27th Dec 1994, Nick Thompson, DEVSUPPORT#include <Files.h>#include <QuickDraw.h>#include <QDOffScreen.h>#include <StandardFile.h>#include "MetaFileReadSupport.h"#include "QD3D.h"#include "QD3DDrawContext.h"#include "QD3DRenderer.h"#include "QD3DShader.h"#include "QD3DCamera.h"#include "QD3DLight.h"#include "QD3DGeometry.h"#include "QD3DTransform.h"#include "QD3DGroup.h"#include "QD3DMath.h"#include "QD3DStorage.h"#include "QD3DIO.h"//-----------------------------------------------------------------------------------------------// local utility functionsstatic	TQ3FileObject 		MyGetNewFile( FSSpec *myFSSpec, TQ3Boolean *isText ) ;void GetGroupBBox(	DocumentPtr			theDocument,	TQ3BoundingBox 		*viewBBox) ;												static	TQ3Status MyAddShaderToGroup( TQ3GroupObject group ) ;static TQ3Status GetDocumentGroupBoundingBox( 	DocumentPtr theDocument , 	TQ3BoundingBox *viewBBox) ;//-----------------------------------------------------------------------------------------------// Submit the scene for rendering/fileIO and pickingTQ3Status SubmitScene( DocumentPtr theDocument ) {			TQ3Vector3D				globalScale;	TQ3Vector3D				globalTranslate;		globalScale.x = globalScale.y = globalScale.z = theDocument->fGroupScale;	globalTranslate = *(TQ3Vector3D *)&theDocument->fGroupCenter;	Q3Vector3D_Scale(&globalTranslate, -1, &globalTranslate);	Q3Style_Submit(theDocument->fInterpolation, theDocument->fView);	Q3Style_Submit(theDocument->fBackFacing , theDocument->fView);	Q3Style_Submit(theDocument->fFillStyle, theDocument->fView);			Q3MatrixTransform_Submit( &theDocument->fRotation, theDocument->fView);			Q3ScaleTransform_Submit(&globalScale, theDocument->fView);	Q3TranslateTransform_Submit(&globalTranslate, theDocument->fView);	Q3DisplayGroup_Submit( theDocument->fModel, theDocument->fView);		return kQ3Success ;}//-----------------------------------------------------------------------------------------------static TQ3Status GetDocumentGroupBoundingBox( 	DocumentPtr theDocument , 	TQ3BoundingBox *viewBBox){	TQ3Status		status;	TQ3ViewStatus	viewStatus ;		status = Q3View_StartBoundingBox( theDocument->fView, kQ3ComputeBoundsApproximate );	do {		status = SubmitScene( theDocument ) ;	} while((viewStatus = Q3View_EndBoundingBox( theDocument->fView, viewBBox )) == kQ3ViewStatusRetraverse );	return status ;}//-----------------------------------------------------------------------------------------------TQ3ViewObject MyNewView(WindowPtr theWindow){	TQ3Status				myStatus;	TQ3ViewObject			myView;	TQ3DrawContextObject		myDrawContext;	TQ3RendererObject		myRenderer;	TQ3CameraObject			myCamera;	TQ3GroupObject			myLights;		myView = Q3View_New();		//	Create and set draw context.	if ((myDrawContext = MyNewDrawContext(theWindow)) == nil )		goto bail;			if ((myStatus = Q3View_SetDrawContext(myView, myDrawContext)) == kQ3Failure )		goto bail;	Q3Object_Dispose( myDrawContext ) ;		// Create and set renderer.	//	// hacky way to do this, but since I wanted these snippets to have 	// a minimal interface, this will suffice	//	// change the next line to �#if 1� to use the WF renderer	#if 0	// this would use the wireframe renderer	myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeWireFrame);	if ((myStatus = Q3View_SetRenderer(myView, myRenderer)) == kQ3Failure ) {		goto bail;	}#else	// this would use the interactive software renderer	if ((myRenderer = Q3Renderer_NewFromType(kQ3RendererTypeInteractive)) != nil ) {		if ((myStatus = Q3View_SetRenderer(myView, myRenderer)) == kQ3Failure ) {			goto bail;		}	}	else {		goto bail;	}#endif	Q3Object_Dispose( myRenderer ) ;		//	Create and set camera.	if ( (myCamera = MyNewCamera(theWindow)) == nil )		goto bail;			if ((myStatus = Q3View_SetCamera(myView, myCamera)) == kQ3Failure )		goto bail;	Q3Object_Dispose( myCamera ) ;		//	Create and set lights.	if ((myLights = MyNewLights()) == nil )		goto bail;			if ((myStatus = Q3View_SetLightGroup(myView, myLights)) == kQ3Failure )		goto bail;			Q3Object_Dispose(myLights);	//	Done!!!	return ( myView );	bail:	//	If any of the above failed, then don't return a view.	return ( nil );}//----------------------------------------------------------------------------------TQ3DrawContextObject MyNewDrawContext(WindowPtr theWindow){	TQ3DrawContextData		myDrawContextData;	TQ3MacDrawContextData	myMacDrawContextData;	TQ3ColorARGB			ClearColor;	TQ3DrawContextObject	myDrawContext ;		ClearColor.a = 1.0;	ClearColor.r = 1.0;	ClearColor.g = 1.0;	ClearColor.b = 1.0;		//	Fill in draw context data.	myDrawContextData.clearImageMethod = kQ3ClearMethodWithColor;	myDrawContextData.clearImageColor = ClearColor;		myDrawContextData.paneState = kQ3False;	myDrawContextData.maskState = kQ3False;		myDrawContextData.doubleBufferState = kQ3True;	myMacDrawContextData.drawContextData = myDrawContextData;		myMacDrawContextData.window = (CGrafPtr) theWindow;		// this is the window associated with the view	myMacDrawContextData.library = kQ3Mac2DLibraryNone;	myMacDrawContextData.viewPort = nil;	myMacDrawContextData.grafPort = nil;		//	Create draw context and return it, if it�s nil the caller must handle	myDrawContext = Q3MacDrawContext_New(&myMacDrawContextData) ;	return myDrawContext ;}//----------------------------------------------------------------------------------TQ3CameraObject MyNewCamera(WindowPtr theWindow){	TQ3CameraObject					myCamera;	TQ3CameraData					myCameraData;	TQ3ViewAngleAspectCameraData		myViewAngleCameraData;	TQ3Point3D						cameraFrom 	= { 0.0, 0.0, 30.0 };	TQ3Point3D						cameraTo 	= { 0.0, 0.0, 0.0 };	TQ3Vector3D						cameraUp 	= { 0.0, 1.0, 0.0 };		float 							fieldOfView = .52359333333;	float 							hither 		= 0.001;	float 							yon 		= 1000;		//	Fill in camera data.	myCameraData.placement.cameraLocation = cameraFrom;	myCameraData.placement.pointOfInterest = cameraTo;	myCameraData.placement.upVector = cameraUp;		myCameraData.range.hither = hither;	myCameraData.range.yon = yon;		myCameraData.viewPort.origin.x = -1.0;	myCameraData.viewPort.origin.y = 1.0;	myCameraData.viewPort.width = 2.0;	myCameraData.viewPort.height = 2.0;		myViewAngleCameraData.cameraData = myCameraData;	myViewAngleCameraData.fov = fieldOfView ;		// set up the aspect ratio based on the window	myViewAngleCameraData.aspectRatioXToY =  			(float) (theWindow->portRect.right - theWindow->portRect.left) / 			(float) (theWindow->portRect.bottom - theWindow->portRect.top);	myCamera = Q3ViewAngleAspectCamera_New(&myViewAngleCameraData);			//	Return the camera.	return ( myCamera );}//----------------------------------------------------------------------------------TQ3GroupObject MyNewLights(){	TQ3GroupPosition			myGroupPosition;	TQ3GroupObject			myLightList;	TQ3LightData				myLightData;	TQ3PointLightData		myPointLightData;	TQ3DirectionalLightData	myDirectionalLightData;	TQ3LightObject			myAmbientLight, myPointLight, myFillLight;	TQ3Point3D				pointLocation = { -10.0, 0.0, 10.0 };	TQ3Vector3D				fillDirection = { 10.0, 0.0, 10.0 };	TQ3ColorRGB				WhiteLight = { 1.0, 1.0, 1.0 };		//	Set up light data for ambient light.  This light data will be used for point and fill	//	light also.	myLightData.isOn = kQ3True;	myLightData.color = WhiteLight;		//	Create ambient light.	myLightData.brightness = .2;	myAmbientLight = Q3AmbientLight_New(&myLightData);	if ( myAmbientLight == nil )		goto bail;		//	Create point light.	myLightData.brightness = 1.0;	myPointLightData.lightData = myLightData;	myPointLightData.castsShadows = kQ3False;	myPointLightData.attenuation = kQ3AttenuationTypeNone;	myPointLightData.location = pointLocation;	myPointLight = Q3PointLight_New(&myPointLightData);	if ( myPointLight == nil )		goto bail;	//	Create fill light.	myLightData.brightness = .2;	myDirectionalLightData.lightData = myLightData;	myDirectionalLightData.castsShadows = kQ3False;	myDirectionalLightData.direction = fillDirection;	myFillLight = Q3DirectionalLight_New(&myDirectionalLightData);	if ( myFillLight == nil )		goto bail;	//	Create light group and add each of the lights into the group.	myLightList = Q3LightGroup_New();	if ( myLightList == nil )		goto bail;	myGroupPosition = Q3Group_AddObject(myLightList, myAmbientLight);	if ( myGroupPosition == 0 )		goto bail;	myGroupPosition = Q3Group_AddObject(myLightList, myPointLight);	if ( myGroupPosition == 0 )		goto bail;	myGroupPosition = Q3Group_AddObject(myLightList, myFillLight);	if ( myGroupPosition == 0 )		goto bail;	Q3Object_Dispose( myAmbientLight ) ;	Q3Object_Dispose( myPointLight ) ;	Q3Object_Dispose( myFillLight ) ;	//	Done!	return ( myLightList );	bail:	//	If any of the above failed, then return nothing!	return ( nil );}//----------------------------------------------------------------------------------TQ3GroupObject MyNewModelFromFile(FSSpec *theFileSpec){	TQ3GroupObject		myGroup = NULL ;	TQ3Boolean			isText = kQ3False ;	TQ3FileMode			myFileMode = 0;	TQ3FileObject		theFile;		//	Create a ordered group for the complete model.	if ((myGroup = Q3DisplayGroup_New()) == NULL )		return NULL;			MyAddShaderToGroup( myGroup ) ;	theFile = MyGetNewFile( theFileSpec, &isText ) ;		if( isText == kQ3True )		myFileMode |= kQ3FileModeText;	// is it a text metafile??		// Open the file object	if( Q3File_OpenRead( theFile, &myFileMode ) != kQ3Success)		return  NULL ;	if( MyReadModelFromFile( theFile, myGroup ) == 0)		DebugStr("\pMetafile data read is null") ;		Q3File_Close(theFile);			// close and dispose of the file object	Q3Object_Dispose(theFile);		return myGroup ;}//----------------------------------------------------------------------------------// attach a shader to the groupTQ3Status MyAddShaderToGroup( TQ3GroupObject group ){	TQ3ShaderObject	illuminationShader = Q3PhongIllumination_New();	Q3Group_AddObject(group, illuminationShader);	Q3Object_Dispose(illuminationShader);	return(kQ3Success);}//----------------------------------------------------------------------------------// read model from file object into the supplied groupTQ3Status MyReadModelFromFile( TQ3FileObject theFile,TQ3GroupObject myGroup){		if(theFile != NULL) {			TQ3Object			myTempObj ;		TQ3Boolean			isEOF ;							// read objects from the file		do {					myTempObj = Q3File_ReadObject( theFile );						if( myTempObj != NULL ) {				// we only want the object in our main group if we can draw it				if ( Q3Object_IsDrawable( myTempObj) ) 					Q3Group_AddObject( myGroup, myTempObj ) ;								// we either added the object to the main group, or we don't care				// so we can safely dispose of the object				Q3Object_Dispose( myTempObj ) ;			}						// check to see if we reached the end of file yet			isEOF = Q3File_IsEndOfFile( theFile );					} while (isEOF == kQ3False);		}		if( myGroup != NULL )		return kQ3Success ;	else		return kQ3Failure ;}//-----------------------------------------------------------------------------------------------// cleaned up from IM QuickDraw 3D pp 15-5static TQ3FileObject MyGetNewFile( FSSpec *myFSSpec, TQ3Boolean *isText ){	TQ3FileObject		myFileObj;	TQ3StorageObject		myStorageObj;	OSType				myFileType;		FInfo				fndrInfo ;	// we assume the FSSpec passed in was valid, get the file information	// we need to know the file type, this routine may get called by an appleEvent	// handler, so we can't assume a type, we need to get it from the fsspec.		FSpGetFInfo( myFSSpec, &fndrInfo ) ;		// pull out the file type		myFileType = fndrInfo.fdType ;		// Create new storage object and new file object 	if(((myStorageObj = Q3FSSpecStorage_New( myFSSpec )) == NULL) 		|| ((myFileObj = Q3File_New()) == NULL)) 	{		if (myStorageObj != NULL) 			Q3Object_Dispose(myStorageObj);		return(NULL);	}	// Set the storage for the file object	Q3File_SetStorage(myFileObj, myStorageObj);	Q3Object_Dispose(myStorageObj);	if (myFileType == '3DMF')		*isText = kQ3False ;	else if (myFileType == 'TEXT')		*isText = kQ3True ;	return (myFileObj);}//-------------------------------------------------------------------------------------------//Boolean MetafileFileSpecify( FSSpec *theFile ){	StandardFileReply	theSFReply ;	SFTypeList			myTypes = { '3DMF' } ;	const short			numTypes = 1 ;			// Get the file name to open	StandardGetFile( nil, numTypes, myTypes, &theSFReply ) ;		if( theSFReply.sfGood )		*theFile = theSFReply.sfFile ;		// did the user cancel?	return theSFReply.sfGood ;	}//----------------------------------------------------------------------------------void GetGroupBBox(	DocumentPtr			theDocument,	TQ3BoundingBox 		*viewBBox){	TQ3Point3D 					from 	= { 0.0, 0.0, 1.0 };	TQ3Point3D 					to 		= { 0.0, 0.0, 0.0 };	TQ3Vector3D 				up 		= { 0.0, 1.0, 0.0 };		float 						fieldOfView = .52359333333;	float 						hither 		=  0.5;	float 						yon 		=  1.5;	TQ3GroupObject				mainGroup = theDocument->fModel ;	TQ3Status					status;	#ifdef BETA_1_BUILD	Q3View_StartBounds( theDocument->fView );	status = Q3DisplayGroup_BoundingBox(mainGroup, 										viewBBox, 										kQ3ComputeBoundsApproximate,								 	    viewObject);	Q3View_EndBounds( theDocument->fView );#else	status = GetDocumentGroupBoundingBox( theDocument , viewBBox) ;#endif								        	//	//  If we have a point model, then the "viewBBox" would end up	//  being a "singularity" at the location of the point.  As	//  this bounding "box" is used in setting up the camera spec,	//  we get bogus input into Escher.		{ 		float		xSize, ySize, zSize;				xSize = viewBBox->max.x - viewBBox->min.x;		ySize = viewBBox->max.y - viewBBox->min.y;		zSize = viewBBox->max.z - viewBBox->min.z;		if (xSize <= kQ3RealZero &&		    ySize <= kQ3RealZero &&			zSize <= kQ3RealZero) {						viewBBox->max.x += 0.0001;			viewBBox->max.y += 0.0001;			viewBBox->max.z += 0.0001;						viewBBox->min.x -= 0.0001;			viewBBox->min.y -= 0.0001;			viewBBox->min.z -= 0.0001;		}	}}//------------------------------------------------------------------------TQ3Point3D AdjustCamera(	DocumentPtr			theDocument,	short				winWidth,	short				winHeight){	float 						fieldOfView;	float 						hither;	float 						yon;	TQ3CameraPlacement			placement;	TQ3CameraRange				range;	TQ3BoundingBox 				viewBBox;	long 						fromAxis;		float 						maxDimension; 	float						xSize, ySize, zSize;	float						weights[2] = { 0.5, 0.5 };	TQ3Point3D					points[2];	TQ3Vector3D				 	viewVector;	TQ3Vector3D					normViewVector;	TQ3Vector3D					eyeToFrontClip;	TQ3Vector3D					eyeToBackClip;	float						viewDistance;	TQ3Vector3D					diagonalVector;	float						ratio;	TQ3CameraObject				camera;		TQ3ViewObject				theView = theDocument->fView ;	TQ3GroupObject				mainGroup = theDocument->fModel ;		TQ3Point3D					*documentGroupCenter = &theDocument->fGroupCenter ;	float						*documentGroupScale  = &theDocument->fGroupScale ;	Q3View_GetCamera( theView, &camera);	GetGroupBBox( theDocument, &viewBBox);	/*	 *  If we have a point model, then the "viewBBox" would end up	 *  being a "singularity" at the location of the point.  As	 *  this bounding "box" is used in setting up the camera spec,	 *  we get bogus input into Escher.	 */	xSize = viewBBox.max.x - viewBBox.min.x;	ySize = viewBBox.max.y - viewBBox.min.y;	zSize = viewBBox.max.z - viewBBox.min.z;	if (xSize <= kQ3RealZero &&	    ySize <= kQ3RealZero &&		zSize <= kQ3RealZero)  {		viewBBox.max.x += 0.0001;		viewBBox.max.y += 0.0001;		viewBBox.max.z += 0.0001;				viewBBox.min.x -= 0.0001;		viewBBox.min.y -= 0.0001;		viewBBox.min.z -= 0.0001;	}	points[0] = viewBBox.min;	points[1] = viewBBox.max;	Q3Point3D_AffineComb(points, weights, 2, documentGroupCenter);	/*	 *  The "from" point is on a vector perpendicular to the plane	 *  in which the bounding box has greatest dimension.  As "up" is	 *  always in the positive y direction, look at x and z directions.	 */	xSize = viewBBox.max.x - viewBBox.min.x;	zSize = viewBBox.max.z - viewBBox.min.z;		if (xSize > zSize) {		fromAxis = kQ3AxisZ;	} else {		fromAxis = kQ3AxisX;	}	/*	 *  Compute the length of the diagonal of the bounding box.	 *	 *  The hither and yon planes are adjusted so that the 	 *  diagonal of the bounding box is 7/8 the size of the 	 *  minimum dimension of the view frustum. The diagonal is used instead 	 *  of the maximum size (in x, y, or z) so that when you rotate 	 *  the object, the corners don't get clipped out. 	 */	Q3Point3D_Subtract(		&viewBBox.max,		&viewBBox.min,		&diagonalVector);	maxDimension	=	Q3Vector3D_Length(&diagonalVector);	maxDimension	*=	8.0 / 7.0;		ratio = 1.0 / maxDimension;				*documentGroupScale = ratio;		Q3Camera_GetPlacement(camera, &placement);	Q3Point3D_Subtract(		&placement.cameraLocation,		&placement.pointOfInterest,		&viewVector);			viewDistance = Q3Vector3D_Length(&viewVector);		Q3Vector3D_Normalize(&viewVector, &normViewVector);		Q3Vector3D_Scale(&normViewVector, 					 viewDistance - ratio * maxDimension/2.0,					 &eyeToFrontClip);						Q3Vector3D_Scale(&normViewVector, 					viewDistance + ratio * maxDimension/2.0,					&eyeToBackClip);	hither 	= Q3Vector3D_Length(&eyeToFrontClip);	yon 	= Q3Vector3D_Length(&eyeToBackClip);		fieldOfView = 2 * atan((ratio * maxDimension/2.0)/hither);	range.hither 				= hither;	range.yon 					= yon;	Q3Camera_SetRange(camera, &range);	Q3ViewAngleAspectCamera_SetFOV(		camera, fieldOfView);	Q3ViewAngleAspectCamera_SetAspectRatio(		camera, (float) winWidth / (float) winHeight);	Q3Object_Dispose(camera);		return( *documentGroupCenter );}