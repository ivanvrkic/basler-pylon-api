/*
 * UniZG - FER
 * University of Zagreb (http://www.unizg.hr/)
 * Faculty of Electrical Engineering and Computing (http://www.fer.unizg.hr/)
 * Unska 3, HR-10000 Zagreb, Croatia
 *
 * (c) 2015-2017 UniZG, Zagreb. All rights reserved.
 * (c) 2015-2017 FER, Zagreb. All rights reserved.
 */

/*!
  \file   BatchAcquisitionVTK.h
  \brief  VTK viewer window.

  Header for VTK view window.

  \author Tomislav Petkovic
  \date   2017-05-15
*/


#ifndef __BATCHACQUISITIONVTK_H
#define __BATCHACQUISITIONVTK_H


/* The include order may vary so predeclare typedefs. */
struct VTKviewpoint_;
struct VTKsurfacedata_;
struct VTKoutlinedata_;
struct VTKwindowdata_;
struct VTKpointclouddata_;
struct VTKdisplaythreaddata_;


#include "BatchAcquisition.h"
#include "BatchAcquisitionProcessing.h"


// Pre-declare classes.
class CustomInteractorStyle;
class DynamicRangeThresholdCallback;
class AlignedLineRepresentation;
class AlignedLineCallback;
class AllAlignedLinesCallback;
class SlicingPlaneCallback;


//! Indicates slicing plane.
/*!
  Slicing plane for the data may be axial, coronal, or sagittal.
*/
typedef
enum SlicingPlane_
  {
    VTK_PLANE_AXIAL, /*!< Axial slicing plane. Free coordinate is on y axis. */
    VTK_PLANE_CORONAL, /*!< Coronal slicing plane. Free coordinate is on z axis. */
    VTK_PLANE_SAGITTAL, /*!< Sagittal slicing plane. Free coordinate is on x axis.  */
    VTK_PLANE_UNKNOWN /*!< Unknown slicing plane. */
  } SlicingPlane;


//! Indicates threshold control.
/*!
  Threshold slider may control several values to filter out the points.
*/
typedef
enum ThresholdControl_
  {
    VTK_THRESHOLD_RANGE, /*!< Threshold controls dynamic range. */
    VTK_THRESHOLD_RAY_DISTANCE, /*!< Threshold controls ray-to-ray distance. */
    VTK_THRESHOLD_PHASE_DISTANCE, /*!< Threshold controls distance-to-constellation or distance to median. */
    VTK_THRESHOLD_PHASE_DEVIATION, /*!< Threshold controls standard deviation of phase. */
    VTK_THRESHOLD_UNKNOWN /*!< Unknown threshold control. */
  } ThresholdControl;


//! Indicates visibility status.
/*!
  Actors in VTK may be fully transparent, fully opaque, or in-between.
*/
typedef
enum VisibilityStatus_
  {
    VTK_ACTOR_INVISIBLE, /*!< Indicates the actor should be totaly transparent (invisibel). */
    VTK_ACTOR_TRANSPARENT, /*!< Indicates the actor should use preset transparency value. */
    VTK_ACTOR_OPAQUE, /*!< Indicates the actor is fully opaque. */
    VTK_ACTOR_VISIBILITY_UNDEFINED, /*!< Unknown visibility status. */
  } VisibilityStatus;


/****** DATA STRUCTURES ******/

//! Three element point or vector.
/*!
  Three element point or vector of the double type (double precision).
*/
typedef struct VTK3tuple_
{
  double x; //!< First component.
  double y; //!< Second component.
  double z; //!< Third component.
} VTK3tuple;


//! Structure holds pipeline for surface display.
/*!
  To visualize the surface we need to create a VTK visualization
  pipeline that consists of several objects. All required objects
  are collected into this sturcture. Once the pipeline
  is no longer needed we must delete all objects.
*/
typedef struct VTKsurfacedata_
{
  vtkSurfaceReconstructionFilter * surfaceExtractor; /*!< VTK surface reconstructor. */
  vtkContourFilter * surfaceFilter; /*!< VTK filter to extract the surface. */
  vtkPolyDataMapper * surfaceMapper; /*!< VTK mapper to map the surface. */
  vtkActor * surfaceActor; /*!< Actor for the extracted surface. */
} VTKsurfacedata;



//! Structure holds pipeline for surface outline display.
/*!
  To visualize the surface we need to create a VTK visualization
  pipeline that consists of several objects. All required objects
  are collected into this sturcture. Once the pipeline
  is no longer needed we must delete all objects.
*/
typedef struct VTKoutlinedata_
{
  vtkOutlineFilter * outlineExtractor; /*!< VTK outline filter to extract wireframe outline. */
  vtkPolyDataMapper * outlineMapper; /*!< VTK mapper to map the wireframe. */
  vtkActor * outlineActor; /*!< Actor for the extracted wireframe. */
} VTKoutlinedata;



//! Structure holding the visualization pipeline for the point cloud data.
/*!
  To visualize the point cloud data we need to create a VTK visualization
  pipeline. All required objects for the point cloud visualization are collected
  into this structure. Once the visualization pipeline is no longer needed all
  objects must be deleted.
*/
typedef struct VTKpointclouddata_
{
  int CameraID; /*!< Camera ID of the point cloud data. */
  int ProjectorID; /*!< *Projector ID of the point cloud data. */

  double cmx; /*!< Center of mass on x coordinate. */
  double cmy; /*!< Center of mass on y coordinate. */
  double cmz; /*!< Center of mass on z coordinate. */

  double mdx; /*!< X coordinate of the median of the point cloud. */
  double mdy; /*!< Y coordinate of the median of the point cloud. */
  double mdz; /*!< Z coordinate of the median of the point cloud. */

  float rangeMin; /*!< Minumum value of dynamic range. */
  float rangeThr; /*!< Value of the dynamic range threshold. */
  float rangeMax; /*!< Maximum value of dynamic range. */

  float rayDistanceMin; /*!< Minimum value of 3D ray-to-ray distance. */
  float rayDistanceThr; /*!< Value of the 3D ray-to-ray distance threshold. */
  float rayDistanceMax; /*!< Maximum value of 3D ray-to-ray distance. */

  float phaseDistanceMin; /*!< Minimum value of phase distance to constellation. */
  float phaseDistanceThr; /*!< Value of the phase distance to constellation threshold. */
  float phaseDistanceMax; /*!< Maximum value of phase distance to constellation. */

  float phaseDeviationMin; /*!< Minimum value of standard deviation. */
  float phaseDeviationThr; /*!< Value of the standard deviation threshold. */
  float phaseDeviationMax; /*!< Maximum value of standard deviation. */

  float colorScale; /*!< Color scaling factor. */
  float colorOffset; /*!< Color offset. */

  std::wstring * acquisition_name; /*!< Default acquisition name. */

  std::vector<float> * pDynamicRange; /*!< Dynamic range for input points. */
  std::vector<float> * pRayDistance; /*!< Ray-to-ray distance between triangulated 3D rays. */
  std::vector<float> * pPhaseDistance; /*!< Unwrapped phase distance to constellation or distance to histogram median. */
  std::vector<float> * pPhaseDeviation; /*!< Standard deviation of unwrapped phase. */

  std::vector<unsigned char> * pMask; /*!< Opacity mask. */

  ThresholdControl thresholdType; /*!< Currently active threshold control. */

  vtkPoints * Cloud; /*!< Container where we copy the point coordinates for all points. */

  vtkUnsignedCharArray * ColorsMapped; /*!< Container where we stored mapped colors for all points. */
  vtkUnsignedCharArray * ColorsOriginal; /*!< Container where we copy the color for all points. */

  vtkPolyData * CloudPoints; /*!< Container for point data. */
  vtkVertexGlyphFilter * PointsToVertexes; /*!< Filter to produce vertexes for all points. */
  vtkPolyData * CloudVertexes; /*!< Container for colored point data. */

  vtkPolyDataMapper * Mapper; /*!< VTK mapper to map the vertices. */
  vtkActor * Actor; /*!< Actor for the created point cloud.  */

  VTKsurfacedata_ * surface; /*!< Extracted surface. */
  VTKoutlinedata_ * outline; /*!< Extracted outline. */
} VTKpointclouddata;



//! Structure to hold the visualization pipeline for the slicing plane.
/*!
  Structure holds the visualization pipeline for the slicing plane.
*/
typedef struct VTKslicingplane_
{
  double nx; //!< Plane normal in x direction.
  double ny; //!< Plane normal in y direction.
  double nz; //!< Plane normal in z direction.

  double px; //!< X coordinate of one point in the plane.
  double py; //!< Y coordinate of one point in the plane.
  double pz; //!< Z coordinate of one point in the plane.

  double bounds[6]; //!< Bounding box in order: xmin, xmax, ymin, ymax, zmin, and zmax.

  vtkPoints * Points; //!< Polygon endpoints.
  vtkPolygon * Polygon; //!< Polygon to represent the slicing plane.
  vtkCellArray * Polygons; //!< Array of polygons representing the slicing plane.
  vtkPolyData * Plane; //!< Slicing plane representation.
  vtkPolyDataMapper * Mapper; /*!< VTK mapper to map the vertices. */
  vtkActor * Actor; /*!< Actor for the created point cloud.  */
} VTKslicingplane;



//! Structure to store viewpoint.
/*!
  When visualizing we need to select proper camera position that gives exactly the same
  view as either the projector or the camera. To do so we need to know the geometry
  which is stored in this structure. Reference coordinate system we use is one obtained
  during calibration.
*/
typedef struct VTKviewpoint_
{
  vtkCamera * camera3D; /*!< VTK camera class for 3D view. */
  vtkCamera * cameraTop; /*!< Orthographic projection for top view. */
  vtkCamera * cameraFront; /*!< Orthographic projection for front view. */
  vtkCamera * cameraSide; /*!< Orthographic projection for side view. */

  ProjectiveGeometry_ * geometry; /*!< Ideal pinhole camera model. */
} VTKviewpoint;



//! Structure holds VTK display window data.
/*!
  To display the data we need to open VTK visualization window and assign
  several actors to it. We also need interactor to handle the mouse
  input. As there can be only one renderer to ensure no concurent access
  is done we also need a critical section object.
*/
typedef struct VTKwindowdata_
{
  vtkRenderer * ren3D; /*!< Default 3D renderer. */
  vtkRenderer * renTop; /*!< Default renderer for top view. */
  vtkRenderer * renFront; /*!< Default renderer for front view. */
  vtkRenderer * renSide; /*!< Default renderer in for side view. */

  vtkSliderWidget * sldThr; /*!< Slider to control the data threshold. */
  vtkSliderRepresentation2D * sldThrRep; /*!< Representation of the slider to control the threshold. */
  DynamicRangeThresholdCallback * sldThrCallback; /*!< Callback function which adjusts visible points. */

  vtkLineWidget2 * planeAxial1; /*!< Line widget to control the position of axial (transverse) slicing plane. */
  vtkLineWidget2 * planeAxial2; /*!< Line widget to control the position of axial (transverse) slicing plane. */
  vtkLineWidget2 * planeCoronal1; /*!< Line widget to control the position of coronal (front) slicing plane. */
  vtkLineWidget2 * planeCoronal2; /*!< Line widget to control the position of coronal (front) slicing plane. */
  vtkLineWidget2 * planeSagittal1; /*!< Line widget to control the position of sagittal slicing plane. */
  vtkLineWidget2 * planeSagittal2; /*!< Line widget to control the position of sagittal slicing plane. */

  AlignedLineRepresentation * representationAxial1; /*!< Line representation to control the position of axial (transverse) slicing plane. */
  AlignedLineRepresentation * representationAxial2; /*!< Line representation to control the position of axial (transverse) slicing plane. */
  AlignedLineRepresentation * representationCoronal1; /*!< Line representation to control the position of coronal (front) slicing plane. */
  AlignedLineRepresentation * representationCoronal2; /*!< Line representation to control the position of coronal (front) slicing plane. */
  AlignedLineRepresentation * representationSagittal1; /*!< Line representation to control the position of sagittal slicing plane. */
  AlignedLineRepresentation * representationSagittal2; /*!< Line representation to control the position of sagittal slicing plane. */

  AlignedLineCallback * callbackAxial1; /*!< Callback to match remaining axial planes to the first one. */
  AlignedLineCallback * callbackAxial2; /*!< Callback to match remaining axial planes to the second one. */
  AlignedLineCallback * callbackCoronal1; /*!< Callback to match remaining coronal planes to the first one. */
  AlignedLineCallback * callbackCoronal2; /*!< Callback to match remaining coronal planes to the second one. */
  AlignedLineCallback * callbackSagittal1; /*!< Callback to match remaining sagittal planes to the first one. */
  AlignedLineCallback * callbackSagittal2; /*!< Callback to match remaining sagittal planes to the second one. */

  AllAlignedLinesCallback * callbackViewpointChange; /*!< Callback to fix line positions when viewpoint changes. */

  VTKslicingplane * planeAxial; /*!< Axial slicing plane. */
  VTKslicingplane * planeCoronal; /*!< Coronal slicing plane. */
  VTKslicingplane * planeSagittal; /*!< Sagittal slicing plane. */

  SlicingPlaneCallback * callbackAxial; /*!< Callback to match the axial plane position to plane control widget state. */
  SlicingPlaneCallback * callbackCoronal; /*!< Callback to match the coronal plane position to plane control widget state. */
  SlicingPlaneCallback * callbackSagittal; /*!< Callback to match the sagittal plane position to plane control widget state. */

  vtkTextActor * slicingStatistics; /*!< Actor to add slicing statistics to the display. */

  vtkRenderWindow * renWin; /*!< Default renderer window. */
  vtkRenderWindowInteractor * renWinInt; /*!< Default mouse interactor. */
  CustomInteractorStyle * renWinIntStyle; /*!< Default mouse event handler. */

  vtkCallbackCommand * pushCallback; /*!< Default render callback to update pushed data. */
  vtkCallbackCommand * popCallback; /*!< Default render callback to pop data. */
  vtkCallbackCommand * keypressCallback; /*!< Default interactor callback to restore viewpoint. */

  VisibilityStatus slicing_planes_visibility; /*!< Visibility status of slicing planes. */
  
  volatile bool interactorRunning; /*!< Flag to indicate wheather interactor is active or not. */

  CRITICAL_SECTION rendererCS; /*!< Critical section used to coordinate access to the renderer. */
} VTKwindowdata;



//! Structure to hold VTK display thread data.
/*!
  When displaying the data the display is driven by a separate thread which runs message pump and VTK callbacks.
  As the window interactor loop is always active we update the model via callbacks.
  A critical section pushCS is used to control concurrent access to variables.

  To reduce the overhead all data push function create data structures using the variables whose name ends with NEW.
  The data is periodically copied from variables named *NEW to corresponding variables without NEW in the name via a callback.
  Therefore the pushCS critical section is only used to control the access to *NEW variables.
*/
typedef struct VTKdisplaythreaddata_
{
  HANDLE thread; /*!< Thread handle. */
  DWORD threadID; /*!< Thread ID. */

  VTKwindowdata_ * window; /*!< Display window data. */

  VTKviewpoint_ * camera; /*!< Currently active VTK camera. */
  VTKviewpoint_ * cameraNEW; /*!< New VTK camera to be activated. */

  std::vector<VTKpointclouddata_ *> * point_clouds; /*!< All point clouds that are currently displayed. */
  std::vector<VTKpointclouddata_ *> * point_cloudsNEW; /*!< New point clouds to be displayed. */

  std::vector<ProjectiveGeometry_ *> * projector_geometries; /*!< Projector view geometries. */
  std::vector<ProjectiveGeometry_ *> * projector_geometriesNEW; /*!< New projector view geometries to be activated. */
  
  std::vector<ProjectiveGeometry_ *> * camera_geometries; /*!< Camera view geometries. */
  std::vector<ProjectiveGeometry_ *> * camera_geometriesNEW; /*!< New camera view geometries to be activated. */

  int CloudID; /*!< Index of the active point cloud. */
  int CameraID; /*!< Camera ID of the active camera. */
  int ProjectorID; /*!< Projector ID of the active projector. */

  volatile bool camera_pushed; /*!< Flag to indicate new VTK camera data was pushed. */  
  volatile bool point_cloud_pushed; /*!< Flag to indicate we pushed a new point cloud data to display. */
  volatile bool projector_geometry_pushed; /*!< Flag to indicate we pushed new projector geometry to display. */
  volatile bool camera_geometry_pushed; /*!< Flag to indicate we pushed new camera geometry to display. */

  volatile bool clear_all; /*!< Flag to indicate all pushed data should be cleared. */

  volatile bool terminate; /*!< Flag to indicate VTK dispaly thread should terminate. */

  CRITICAL_SECTION dataCS; /*!< Thread data critical section. */
  CRITICAL_SECTION pushCS; /*!< Thread push critical section. */

  VTKdisplaythreaddata_ * myAddress; /*!< Address of this structure. */
} VTKdisplaythreaddata;



/****** CLASSES ******/

//! Class override.
/*!
  We override the default vtkInteractorStyleTrackballCamera class as the
  render window contains several viewports which require different interactor
  styles.

  Furtermore, vtkInteractorStyleTrackballCamera defines several interactive
  keys that produce unwanted warning messages. This class is used to override
  the default keyboard handler where we stop unwanted events.
*/
class CustomInteractorStyle : public vtkInteractorStyleTrackballCamera
{
 public:

  //! Static new method to create class instances.
  static CustomInteractorStyle * New();

  vtkTypeMacro(CustomInteractorStyle, vtkInteractorStyleTrackballCamera);

  //! Override default keypress handler.
  virtual void OnChar();

  //! Overrride default state handler.
  virtual void StartState(int);

  //! Override default rotate handler.
  virtual void Rotate();

  //! Override deafult spin handler.
  virtual void Spin();

  //! Override default dolly handler.
  virtual void Dolly(double);

  bool limit_to_2D; //!< Flag to indicate if interaction style is limited to 2D.
  double border_x; //!< Horizontal window border.
};



//! Class to handle threshold changes.
/*!
  This class serves to hold a callback function when threshold changes.
*/
class DynamicRangeThresholdCallback : public vtkCommand
{

 public:

  //! Default constructor.
  /*!
    Method to construct the class.
  */
  static DynamicRangeThresholdCallback * New()
  {
    return new DynamicRangeThresholdCallback;
  }
  /* New */

 DynamicRangeThresholdCallback():D(NULL) {}

  //! Callback to update the visible points in the point cloud.
  virtual void Execute(vtkObject *, unsigned long, void *);

  VTKdisplaythreaddata * D; /*!< Pointer to currently active VTK thread. */
};
/* DynamicRangeThresholdCallback */



//! Class to represent line widget.
/*!
  This class modified normal vtkLineRepresentation to a form aligned
  with the specified axis.
*/
class AlignedLineRepresentation : public vtkLineRepresentation
{
 public:

  //! Static new method to create class instances.
  static AlignedLineRepresentation * New();

  vtkTypeMacro(AlignedLineRepresentation, vtkInteractorStyleTrackballCamera);

  //! Override interaction method.
  void StartWidgetInteraction(double [2]);

  //! Override interaction method.
  void WidgetInteraction(double [2]);

  //! Stretch line to cover viewport.
  void StretchLineToCoverViewport(double);

  //! Stretch widget to bounding box.
  void AdjustWidgetPlacement(void);

  //! Sets line parameters.
  void SetLineParameters(double [3], double [3], double [3], double [6], SlicingPlane);

  //! Sets line color.
  void SetLineColor(double, double, double);

  //! Test if mouse is over the line.
  int ComputeInteractionState(int, int, int);

  double ln_pt[3]; //!< Line point.
  double ln_vec[3]; //!< Normalized line direction.
  double start_pt[3]; //!< Line point at interaction start.
  double move_vec[3]; //!< Allowed line translation.
  double bounds[6]; //!< Bounding box.

  double plane_crd; //!< A fixed coordinate of an aligned plane.
  SlicingPlane plane_type; //!< Plane may be axial, sagittal, or coronal.
};
/* AlignedLineRepresentation */



//! Class to handle slicing plane movement.
/*!
  This class serves to hold a callback function when slicing plane position changes.
*/
class AlignedLineCallback : public vtkCommand
{

 public:

  //! Default constructor.
  /*!
    Method to construct the class.
  */
  static AlignedLineCallback * New()
  {
    return new AlignedLineCallback;
  }
  /* New */

 AlignedLineCallback():L(NULL) {}

  //! Callback to update the second line.
  virtual void Execute(vtkObject *, unsigned long, void *);

  AlignedLineRepresentation * L; //!< Pointer to line representation.
};
/* AlignedLineCallback */



//! Class to handle zoom and plan for slicing planes.
/*!
  This class serves to hold a callback function when camera in render window changes.
*/
class AllAlignedLinesCallback : public vtkCommand
{

 public:

  //! Default constructor.
  /*!
    Method to construct the class.
  */
  static AllAlignedLinesCallback * New()
  {
    return new AllAlignedLinesCallback;
  }
  /* New */

 AllAlignedLinesCallback():W(NULL) {}

  //! Callback to update all lines.
  virtual void Execute(vtkObject *, unsigned long, void *);

  VTKwindowdata * W; //!< Pointer to VTK window data.
};
/* AllAlignedLinesCallback */



//! Class to handle slicing plane position change.
/*!
  This class serves to hold a callback function when user changes slicing plane position.
*/
class SlicingPlaneCallback : public vtkCommand
{

 public:

  //! Default constructor.
  /*!
    Method to construct the class.
  */
  static SlicingPlaneCallback * New()
  {
    return new SlicingPlaneCallback;
  }
  /* New */

 SlicingPlaneCallback():P(NULL) {}

  //! Callback to update plane position.
  virtual void Execute(vtkObject *, unsigned long, void *);

  VTKslicingplane * P; //!< Slicing plane representation.
};
/* SlicingPlaneCallback */



/****** AUXILIARY FUNCTIONS ******/

//! Save VTK scene to file.
HRESULT VTKSaveRenderWindowToFile(vtkRenderWindow * const);

//! Update camera to new geometry.
bool
VTKSetCameraToMatchGeometry(
                            vtkCamera * const,
                            ProjectiveGeometry_ * const,
                            double const * const,
                            double const * const,
                            bool const
                            );

//! Update camera to new orthographic projection.
bool
VTKSetOrthographicProjectionCamera(
                                   vtkCamera * const,
                                   double const *,
                                   double const *,
                                   double const *,
                                   double const *,
                                   double const
                                   );

//! Update camera clipping planes.
bool VTKSetCameraClippingPlanes(vtkCamera * const, double const * const);

//! Update camera focal point.
bool VTKSetCameraFocalPoint(vtkCamera * const, double const * const);

//! Destroy point cloud.
void VTKDeletePointCloudData(VTKpointclouddata_ * const);

//! Create point cloud VTK actor for the point data.
VTKpointclouddata_ *
VTKCreatePointCloudData(
                        cv::Mat * const,
                        cv::Mat * const,
                        cv::Mat * const,
                        int const,
                        int const,
                        wchar_t const * const
                        );

//! Destroy slicing plane data.
void VTKDeleteSlicingPlaneData(VTKslicingplane * const);

//! Creates slicing plane data.
VTKslicingplane * VTKCreateSlicingPlaneData(double const * const, double const * const, double const * const);

//! Destroy view point data.
void VTKDeleteViewPointData(VTKviewpoint_ * const);

//! Create view point data.
VTKviewpoint * VTKCreateViewPointData(ProjectiveGeometry_ * const);

//! Adds actor to VTK renderer.
bool VTKAddActorToDisplayWindow(VTKwindowdata * const, vtkProp * const);

//! Removes actor from VTK renderer.
bool VTKRemoveActorFromDisplayWindow(VTKwindowdata * const, vtkProp * const);

//! Toggle actor in VTK renderer.
bool VTKToggleActorInDisplayWindow(VTKwindowdata * const, vtkActor * const);


/****** OPEN AND CLOSE WINDOW ******/

//! Open VTK display thread.
VTKdisplaythreaddata * OpenVTKWindow(cv::Mat * const, ProjectiveGeometry_ * const, ProjectiveGeometry_ * const);

//! Closes VTK display winow.
void CloseVTKWindow(VTKdisplaythreaddata * const);


/****** DATA PUSH ******/

//! Push point cloud to display thread.
bool
VTKPushPointCloudToDisplayThread(
                                 VTKdisplaythreaddata * const,
                                 cv::Mat * const,
                                 cv::Mat * const,
                                 cv::Mat * const,
                                 int const,
                                 int const,
                                 wchar_t const * const
                                 );

//! Push camera geometry to display thread.
bool VTKPushCameraGeometryToDisplayThread(VTKdisplaythreaddata * const, ProjectiveGeometry_ * const, int const);

//! Push projector geometry to display thread.
bool VTKPushProjectorGeometryToDisplayThread(VTKdisplaythreaddata * const, ProjectiveGeometry_ * const, int const);

//! Clear all pushed data.
bool VTKClearAllPushedData(VTKdisplaythreaddata * const);

//! Update VTK display.
void VTKUpdateDisplay(VTKdisplaythreaddata * const P);


#endif /* !__BATCHACQUISITIONVTK_H */
