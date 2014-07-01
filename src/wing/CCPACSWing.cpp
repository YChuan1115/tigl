/*
* Copyright (C) 2007-2013 German Aerospace Center (DLR/SC)
*
* Created: 2010-08-13 Markus Litz <Markus.Litz@dlr.de>
* Changed: $Id$
*
* Version: $Revision$
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/**
* @file
* @brief  Implementation of CPACS wing handling routines.
*/

#include <iostream>
#include <cassert>

#include "CCPACSWing.h"
#include "CCPACSConfiguration.h"
#include "CTiglAbstractSegment.h"
#include "CCPACSWingSegment.h"
#include "CTiglError.h"

#include "BRepOffsetAPI_ThruSections.hxx"
#include "BRepAlgoAPI_Fuse.hxx"
#include "ShapeFix_Shape.hxx"
#include "GProp_GProps.hxx"
#include "BRepGProp.hxx"
#include "BRepAlgoAPI_Cut.hxx"
#include "Bnd_Box.hxx"
#include "BRepBndLib.hxx"
#include "BRepBuilderAPI_Transform.hxx"
#include "BRepAlgoAPI_Common.hxx"
#include "BRepAlgoAPI_Section.hxx"
#include "BOPCol_ListOfShape.hxx"
#include "BOPAlgo_PaveFiller.hxx"
#include <gce_MakeLin.hxx>
#include <gce_MakeRotation.hxx>

namespace tigl
{

namespace
{
    inline double max(double a, double b)
    {
        return a > b? a : b;
    }

    TopoDS_Wire transformToWingCoords(const tigl::CCPACSWingConnection& wingConnection, const TopoDS_Wire& origWire)
    {
        TopoDS_Shape resultWire(origWire);

        // Do section element transformations
        resultWire = wingConnection.GetSectionElementTransformation().Transform(resultWire);

        // Do section transformations
        resultWire = wingConnection.GetSectionTransformation().Transform(resultWire);

        // Do positioning transformations (positioning of sections)
        resultWire = wingConnection.GetPositioningTransformation().Transform(resultWire);

        // Cast shapes to wires, see OpenCascade documentation
        if (resultWire.ShapeType() != TopAbs_WIRE) {
            throw tigl::CTiglError("Error: Wrong shape type in CCPACSWing::transformToAbsCoords", TIGL_ERROR);
        }

        return TopoDS::Wire(resultWire);
    }
}


// Constructor
CCPACSWing::CCPACSWing(CCPACSConfiguration* config)
    : segments(this)
    , componentSegments(this)
    , configuration(config)
    , rebuildFusedSegments(true)
    , rebuildFusedSegWEdge(true)
    , rebuildShells(true)
{
    Cleanup();
}

// Destructor
CCPACSWing::~CCPACSWing(void)
{
    Cleanup();
}

// Invalidates internal state
void CCPACSWing::Invalidate(void)
{
    invalidated = true;
    segments.Invalidate();
    positionings.Invalidate();
}

// Cleanup routine
void CCPACSWing::Cleanup(void)
{
    name = "";
    transformation.SetIdentity();
    translation = CTiglPoint(0.0, 0.0, 0.0);
    scaling     = CTiglPoint(1.0, 1.0, 1.0);
    rotation    = CTiglPoint(0.0, 0.0, 0.0);

    // Calls ITiglGeometricComponent interface Reset to delete e.g. all childs.
    Reset();

    Invalidate();
}

// Builds transformation matrix for the wing
void CCPACSWing::BuildMatrix(void)
{
    transformation.SetIdentity();

    // Step 1: scale the wing around the orign
    transformation.AddScaling(scaling.x, scaling.y, scaling.z);

    // Step 2: rotate the wing
    // Step 2a: rotate the wing around z (yaw   += right tip forward)
    transformation.AddRotationZ(rotation.z);
    // Step 2b: rotate the wing around y (pitch += nose up)
    transformation.AddRotationY(rotation.y);
    // Step 2c: rotate the wing around x (roll  += right tip up)
    transformation.AddRotationX(rotation.x);

    // Step 3: translate the rotated wing into its position
    transformation.AddTranslation(translation.x, translation.y, translation.z);

    backTransformation = transformation.Inverted();
}

// Update internal wing data
void CCPACSWing::Update(void)
{
    if (!invalidated) {
        return;
    }

    BuildMatrix();
    invalidated = false;
    rebuildFusedSegments = true;    // forces a rebuild of all segments with regards to the updated translation
    rebuildShells = true;
}

// Read CPACS wing element
void CCPACSWing::ReadCPACS(TixiDocumentHandle tixiHandle, const std::string& wingXPath)
{
    Cleanup();

    char*       elementPath;
    std::string tempString;

    // Get subelement "name"
    char* ptrName = NULL;
    tempString    = wingXPath + "/name";
    elementPath   = const_cast<char*>(tempString.c_str());
    if (tixiGetTextElement(tixiHandle, elementPath, &ptrName) == SUCCESS) {
        name          = ptrName;
    }

    // Get attribute "uid"
    char* ptrUID = NULL;
    tempString   = "uID";
    elementPath  = const_cast<char*>(tempString.c_str());
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrUID) == SUCCESS) {
        SetUID(ptrUID);
    }

    // Get subelement "parent_uid"
    char* ptrParentUID = NULL;
    tempString         = wingXPath + "/parentUID";
    elementPath        = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS &&
        tixiGetTextElement(tixiHandle, elementPath, &ptrParentUID) == SUCCESS) {

        SetParentUID(ptrParentUID);
    }


    // Get subelement "/transformation/translation"
    tempString  = wingXPath + "/transformation/translation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(translation.x), &(translation.y), &(translation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <translation/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/scaling"
    tempString  = wingXPath + "/transformation/scaling";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(scaling.x), &(scaling.y), &(scaling.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <scaling/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "/transformation/rotation"
    tempString  = wingXPath + "/transformation/rotation";
    elementPath = const_cast<char*>(tempString.c_str());
    if (tixiCheckElement(tixiHandle, elementPath) == SUCCESS) {
        if (tixiGetPoint(tixiHandle, elementPath, &(rotation.x), &(rotation.y), &(rotation.z)) != SUCCESS) {
            throw CTiglError("Error: XML error while reading <rotation/> in CCPACSWing::ReadCPACS", TIGL_XML_ERROR);
        }
    }

    // Get subelement "sections"
    sections.ReadCPACS(tixiHandle, wingXPath);

    // Get subelement "positionings"
    positionings.ReadCPACS(tixiHandle, wingXPath);

    // Get subelement "segments"
    segments.ReadCPACS(tixiHandle, wingXPath);

    // Get subelement "componentSegments"
    componentSegments.ReadCPACS(tixiHandle, wingXPath);

    // Register ourself at the unique id manager
    configuration->GetUIDManager().AddUID(ptrUID, this);

    // Get symmetry axis attribute, has to be done, when segments are build
    char* ptrSym = NULL;
    tempString   = "symmetry";
    if (tixiGetTextAttribute(tixiHandle, const_cast<char*>(wingXPath.c_str()), const_cast<char*>(tempString.c_str()), &ptrSym) == SUCCESS) {
        SetSymmetryAxis(ptrSym);
    }

    Update();
}

// Returns the name of the wing
const std::string& CCPACSWing::GetName(void) const
{
    return name;
}

// Returns the parent configuration
CCPACSConfiguration& CCPACSWing::GetConfiguration(void) const
{
    return *configuration;
}

// Get section count
int CCPACSWing::GetSectionCount(void) const
{
    return sections.GetSectionCount();
}

// Returns the section for a given index
CCPACSWingSection& CCPACSWing::GetSection(int index) const
{
    return sections.GetSection(index);
}

// Get segment count
int CCPACSWing::GetSegmentCount(void) const
{
    return segments.GetSegmentCount();
}

// Returns the segment for a given index
CTiglAbstractSegment & CCPACSWing::GetSegment(const int index)
{
    return (CTiglAbstractSegment &) segments.GetSegment(index);
}

// Returns the segment for a given uid
CTiglAbstractSegment & CCPACSWing::GetSegment(std::string uid)
{
    return (CTiglAbstractSegment &) segments.GetSegment(uid);
}

// Get componentSegment count
int CCPACSWing::GetComponentSegmentCount(void)
{
    return componentSegments.GetComponentSegmentCount();
}

// Returns the segment for a given index
CTiglAbstractSegment & CCPACSWing::GetComponentSegment(const int index)
{
    return (CTiglAbstractSegment &) componentSegments.GetComponentSegment(index);
}

// Returns the segment for a given uid
CTiglAbstractSegment & CCPACSWing::GetComponentSegment(std::string uid)
{
    return (CTiglAbstractSegment &) componentSegments.GetComponentSegment(uid);
}


// Gets the loft of the whole wing with modeled leading edge.
TopoDS_Shape & CCPACSWing::GetLoftWithLeadingEdge(void)
{
    if (rebuildFusedSegWEdge) {
        fusedSegmentWithEdge = BuildFusedSegments(true);
    }
    rebuildFusedSegWEdge = false;
    return fusedSegmentWithEdge;
}


TopoDS_Shape CCPACSWing::ExtendFlap(std::string flapUID, double flapDeflectionPercantage )
{
    std::map<std::string,double> flapMap;
    flapMap[flapUID] =flapDeflectionPercantage;
    return BuildFusedSegmentsWithFlaps(false, flapMap);
}

// Builds a fuse shape of all wing segments with flaps
TopoDS_Shape CCPACSWing::BuildFusedSegmentsWithFlaps(bool splitWingInUpperAndLower, std::map<std::string,double> flapStatus )
{
    // packing all segments togehter in one Compound.
    TopoDS_Compound wingAndFlaps;
    BRep_Builder compoundBuilder;
    compoundBuilder.MakeCompound (wingAndFlaps);

    TopoDS_Shape trailingEdgeLoftCut;
    TopoDS_Shape wingLoftCut;
    for ( int i = 1; i <= GetComponentSegmentCount(); i++ ) {

       CCPACSWingComponentSegment &componentSegment = componentSegments.GetComponentSegment(i);
       CCPACSTrailingEdgeDevices* trailingEdgeDevices = componentSegment.getControlSurfaces().getTrailingEdgeDevices();
       TopoDS_Shape wingLoft;

       for ( int j = trailingEdgeDevices->getTrailingEdgeDeviceCount(); j > 0 ; j-- ) {
            CCPACSTrailingEdgeDevice &trailingEdgeDevice = trailingEdgeDevices->getTrailingEdgeDeviceByID(j);

            if ( !wingCutOutShape.IsNull() ) {
                wingLoft = wingCutOutShape;
            }
            else {
                wingLoft = GetLoft();
            }

            if ( wingCleanShape.IsNull() ) {
                wingCleanShape = wingLoft;
            }

            if ( trailingEdgeDevice.GetLoft().IsNull() ) {
                // box built out of the 4 edge´s of the trailingEdgeDevice outer shape.
                TopoDS_Shape trailingEdgePrism = trailingEdgeDevice.getCutOutShape();

                // create intermediate result for boolean ops
                BOPCol_ListOfShape aLS;
                aLS.Append(wingLoft);
                aLS.Append(trailingEdgePrism);
                BOPAlgo_PaveFiller dsFill;
                dsFill.SetArguments(aLS);
                dsFill.Perform();

                // create common and cut out structure of the wing and the trailingEdgeDevice
                wingLoftCut  = BRepAlgoAPI_Cut(wingLoft, trailingEdgePrism,dsFill);
                trailingEdgeLoftCut = BRepAlgoAPI_Common(wingLoft, trailingEdgePrism, dsFill);
                wingCutOutShape = wingLoftCut;

                trailingEdgeDevice.setLoft(trailingEdgeLoftCut);
            }
            else {
                wingLoftCut = wingCutOutShape;
                trailingEdgeLoftCut = trailingEdgeDevice.GetLoft();
            }

            BRepBuilderAPI_Transform form(trailingEdgeLoftCut,trailingEdgeDevice.getTransformation(flapStatus[trailingEdgeDevice.getUID()]));

            // shape of the trailingEdgeDevice.
            trailingEdgeLoftCut = form.Shape();

            // adding all shapes to one compound.
            compoundBuilder.Add (wingAndFlaps, trailingEdgeLoftCut);
       }
    }

    // If there are now TrailingEdgeDevices, then there is no wingLoftCut.
    if (wingLoftCut.IsNull()) {
        wingLoftCut = GetLoft();
    }

    // adding all shapes to one compound.
    compoundBuilder.Add (wingAndFlaps, wingLoftCut);
    loft = wingAndFlaps;
    return loft;
}

// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetUpperShape(void)
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return upperShape;
}

// Gets the loft of the whole wing.
TopoDS_Shape & CCPACSWing::GetLowerShape(void)
{
    if (rebuildShells) {
        BuildUpperLowerShells();
    }
    rebuildShells = false;
    return lowerShape;
}

TopoDS_Shape CCPACSWing::BuildLoft()
{
    return BuildFusedSegments(true);
}

TopoDS_Shape & CCPACSWing::GetWingWithoutFlaps()
{
    if(wingCutOutShape.IsNull())
    {
        std::map<std::string,double> flapStatus;
        BuildFusedSegmentsWithFlaps(false, flapStatus);
    }
    return wingCutOutShape;
}

// Builds a fused shape of all wing segments
TopoDS_Shape CCPACSWing::BuildFusedSegments(bool splitWingInUpperAndLower)
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generator(Standard_True, Standard_True, Precision::Confusion() );

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSWingConnection& startConnection = segments.GetSegment(i).GetInnerConnection();
        CCPACSWingProfile& startProfile = startConnection.GetProfile();
        TopoDS_Wire startWire = transformToWingCoords(startConnection, startProfile.GetWire());

        generator.AddWire(startWire);
    }

    CCPACSWingConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetOuterConnection();
    CCPACSWingProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endWire = transformToWingCoords(endConnection,endProfile.GetWire());

    generator.AddWire(endWire);
    generator.CheckCompatibility(Standard_False);
    generator.Build();

    return GetWingTransformation().Transform(generator.Shape());
}

// Builds a fused shape of all wing segments
void CCPACSWing::BuildUpperLowerShells()
{
    //@todo: this probably works only if the wings does not split somewere
    BRepOffsetAPI_ThruSections generatorUp(Standard_False, Standard_True, Precision::Confusion() );
    BRepOffsetAPI_ThruSections generatorLow(Standard_False, Standard_True, Precision::Confusion() );

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSWingConnection& startConnection = segments.GetSegment(i).GetInnerConnection();
        CCPACSWingProfile& startProfile = startConnection.GetProfile();
        TopoDS_Wire upperWire, lowerWire;
        upperWire = transformToWingCoords(startConnection, startProfile.GetUpperWire());
        lowerWire = transformToWingCoords(startConnection, startProfile.GetLowerWire());
        generatorUp.AddWire(upperWire);
        generatorLow.AddWire(lowerWire);
    }

    CCPACSWingConnection& endConnection = segments.GetSegment(segments.GetSegmentCount()).GetOuterConnection();
    CCPACSWingProfile& endProfile = endConnection.GetProfile();
    TopoDS_Wire endUpWire, endLowWire;

    endUpWire = transformToWingCoords(endConnection, endProfile.GetUpperWire());
    endLowWire = transformToWingCoords(endConnection, endProfile.GetLowerWire());

    generatorUp.AddWire(endUpWire);
    generatorLow.AddWire(endLowWire);
    generatorLow.Build();
    generatorUp.Build();
    upperShape = GetWingTransformation().Transform(generatorUp.Shape());
    lowerShape = GetWingTransformation().Transform(generatorLow.Shape());
}



// Gets the wing transformation (original wing implementation, but see GetTransformation)
CTiglTransformation CCPACSWing::GetWingTransformation(void)
{
    Update();   // create new transformation matrix if scaling, rotation or translation was changed
    return transformation;
}

// Get the positioning transformation for a given section-uid
CTiglTransformation CCPACSWing::GetPositioningTransformation(std::string sectionUID)
{
    return positionings.GetPositioningTransformation(sectionUID);
}

// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetUpperPoint(int segmentIndex, double eta, double xsi)
{
    return ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetUpperPoint(eta, xsi);
}

// Gets the upper point in absolute (world) coordinates for a given segment, eta, xsi
gp_Pnt CCPACSWing::GetLowerPoint(int segmentIndex, double eta, double xsi)
{
    return  ((CCPACSWingSegment &) GetSegment(segmentIndex)).GetLowerPoint(eta, xsi);
}

// Returns the volume of this wing
double CCPACSWing::GetVolume(void)
{
    TopoDS_Shape& fusedSegments = GetLoft();

    // Calculate volume
    GProp_GProps System;
    BRepGProp::VolumeProperties(fusedSegments, System);
    double myVolume = System.Mass();
    return myVolume;
}

// Get the Transformation object (general interface implementation)
CTiglTransformation CCPACSWing::GetTransformation(void)
{
    return GetWingTransformation();
}

// Sets the Transformation object
void CCPACSWing::Translate(CTiglPoint trans)
{
    CTiglAbstractGeometricComponent::Translate(trans);
    invalidated = true;
    segments.Invalidate();
    componentSegments.Invalidate();
    Update();
}

// Get Translation
CTiglPoint CCPACSWing::GetTranslation(void)
{
    return translation;
}

// Returns the surface area of this wing
double CCPACSWing::GetSurfaceArea(void)
{
    TopoDS_Shape& fusedSegments = GetLoft();

    // Calculate surface area
    GProp_GProps System;
    BRepGProp::SurfaceProperties(fusedSegments, System);
    double myArea = System.Mass();
    return myArea;
}

// Returns the reference area of the wing by taking account the quadrilateral portions
// of each wing segment by projecting the wing segments into the plane defined by the user
double CCPACSWing::GetReferenceArea(TiglSymmetryAxis symPlane)
{
    double refArea = 0.0;

    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        refArea += segments.GetSegment(i).GetReferenceArea(symPlane);
    }
    return refArea;
}


double CCPACSWing::GetWettedArea(TopoDS_Shape parent)
{
    TopoDS_Shape loft = GetLoft();

    TopoDS_Shape wettedLoft = BRepAlgoAPI_Cut(loft, parent);

    GProp_GProps System;
    BRepGProp::SurfaceProperties(wettedLoft, System);
    double wetArea = System.Mass();
    return wetArea;
}


// Returns the lower Surface of a Segment
Handle(Geom_Surface) CCPACSWing::GetLowerSegmentSurface(int index)
{
    return segments.GetSegment(index).GetLowerSurface();
}

// Returns the upper Surface of a Segment
Handle(Geom_Surface) CCPACSWing::GetUpperSegmentSurface(int index)
{
    return segments.GetSegment(index).GetUpperSurface();
}

// sets the symmetry plane for all childs, segments and component segments
void CCPACSWing::SetSymmetryAxis(const std::string& axis)
{
    CTiglAbstractGeometricComponent::SetSymmetryAxis(axis);

    for (int i = 1; i <= segments.GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = segments.GetSegment(i);
        segment.SetSymmetryAxis(axis);
    }

    for (int i = 1; i <= componentSegments.GetComponentSegmentCount(); ++i) {
        CCPACSWingComponentSegment& compSeg = componentSegments.GetComponentSegment(i);
        compSeg.SetSymmetryAxis(axis);
    }
}

double CCPACSWing::GetWingspan()
{
    Bnd_Box boundingBox;
    if (GetSymmetryAxis() == TIGL_NO_SYMMETRY) {
        for (int i = 1; i <= GetSegmentCount(); ++i) {
            TopoDS_Shape& segmentShape = GetSegment(i).GetLoft();
            BRepBndLib::Add(segmentShape, boundingBox);
        }

        Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
        double xw = xmax - xmin;
        double yw = ymax - ymin;
        double zw = zmax - zmin;

        return max(xw, max(yw, zw));
    }
    else {
        for (int i = 1; i <= GetSegmentCount(); ++i) {
            CTiglAbstractSegment& segment = GetSegment(i);
            TopoDS_Shape& segmentShape = segment.GetLoft();
            BRepBndLib::Add(segmentShape, boundingBox);
            TopoDS_Shape segmentMirroredShape = segment.GetMirroredLoft();
            BRepBndLib::Add(segmentMirroredShape, boundingBox);
        }

        Standard_Real xmin, xmax, ymin, ymax, zmin, zmax;
        boundingBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);

        switch (GetSymmetryAxis()){
        case TIGL_X_Y_PLANE:
            return zmax-zmin;
            break;
        case TIGL_X_Z_PLANE:
            return ymax-ymin;
            break;
        case TIGL_Y_Z_PLANE:
            return xmax-ymin;
            break;
        default:
            return ymax-ymin;
        }
    }
}


/**
    * This function calculates location of the quarter of mean aerodynamic chord,
    * and gives the chord lenght as well. It uses the classical method that can
    * be applied to trapozaidal wings. This method is used for each segment.
    * The values are found by taking into account of sweep and dihedral.
    * But the effect of insidance angle is neglected. These values should coincide
    * with the values found with tornado tool.
    */
void  CCPACSWing::GetWingMAC(double& mac_chord, double& mac_x, double& mac_y, double& mac_z)
{
    double A_sum = 0.;
    double cc_mac_sum=0.;
    gp_XYZ cc_mac_sum_p(0., 0., 0.);

    for (int i = 1; i <= segments.GetSegmentCount(); ++i) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(i);
        gp_Pnt innerLeadingPoint   = segment.GetChordPoint(0, 0.);
        gp_Pnt innerTrailingPoint  = segment.GetChordPoint(0, 1.);
        gp_Pnt outerLeadingPoint   = segment.GetChordPoint(1, 0.);
        gp_Pnt innterTrailingPoint = segment.GetChordPoint(1, 1.);

        double distance  = innerLeadingPoint.Distance(innerTrailingPoint);
        double distance2 = outerLeadingPoint.Distance(innterTrailingPoint);

        // points projected to the x == 0 plane
        gp_Pnt point1= gp_Pnt(0.0, innerLeadingPoint.Y(), innerLeadingPoint.Z());
        gp_Pnt point2= gp_Pnt(0.0, outerLeadingPoint.Y(), outerLeadingPoint.Z());
        gp_Pnt point3= gp_Pnt(0.0, innerTrailingPoint.Y(), innerTrailingPoint.Z());
        gp_Pnt point4= gp_Pnt(0.0, innterTrailingPoint.Y(), innterTrailingPoint.Z());

        double len1 = point1.Distance(point2);
        double len2 = point3.Distance(point4);
        double len3 = outerLeadingPoint.Y()  - innerLeadingPoint.Y();
        double len4 = innterTrailingPoint.Y()- innerTrailingPoint.Y();

        double lenght  =(len1+len2)/2.;
        double lenght2 =(len3+len4)/2.;

        double T = distance2/distance;

        double b_mac =lenght*(2*distance2+distance)/(3*(distance2+distance));
        double c_mac =distance-(distance-distance2)/lenght*b_mac;

        gp_Pnt quarterchord  = segment.GetChordPoint(0, 0.25);
        gp_Pnt quarterchord2 = segment.GetChordPoint(1, 0.25);

        double sw_tan   = (quarterchord2.X()-quarterchord.X())/lenght;
        double dihe_sin = (quarterchord2.Z()-quarterchord.Z())/lenght;
        double dihe_cos = lenght2/lenght;

        gp_XYZ seg_mac_p;
        seg_mac_p.SetX(0.25*distance - 0.25*c_mac + b_mac*sw_tan);
        seg_mac_p.SetY(dihe_cos*b_mac);
        seg_mac_p.SetZ(dihe_sin*b_mac);
        seg_mac_p.Add(innerLeadingPoint.XYZ());

        double A =((1. + T)*distance*lenght/2.);

        A_sum += A;
        cc_mac_sum_p += seg_mac_p.Multiplied(A);
        cc_mac_sum   += c_mac*A;
    }

    // compute mac position and chord
    mac_x     = cc_mac_sum_p.X()/A_sum;
    mac_y     = cc_mac_sum_p.Y()/A_sum;
    mac_z     = cc_mac_sum_p.Z()/A_sum;
    mac_chord = cc_mac_sum/A_sum;
}


// Calculates the segment coordinates from global (x,y,z) coordinates
// Returns the segment index of the according segment
// If x,y,z does not belong to any segment, -1 is returned
int CCPACSWing::GetSegmentEtaXsi(const gp_Pnt& point, double& eta, double& xsi, bool& onTop)
{
    // search the segment
    int segmentFound = -1;
    for (int iSeg = 1; iSeg <= GetSegmentCount(); ++iSeg) {
        CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(iSeg);
        if (segment.GetIsOn(point) == true) {
            segmentFound = iSeg;
            break;
        }
    }

    if (segmentFound <= 0) {
        return -1;
    }

    CCPACSWingSegment& segment = (CCPACSWingSegment&) GetSegment(segmentFound);
    segment.GetEtaXsi(point, eta, xsi);

    // TODO: do we need that here?
    onTop = segment.GetIsOnTop(point);

    return segmentFound;
}

// Get the guide curve with a given UID
CCPACSGuideCurve& CCPACSWing::GetGuideCurve(std::string uid)
{
    for (int i=1; i <= segments.GetSegmentCount(); i++) {
        CCPACSWingSegment& segment = segments.GetSegment(i);
        if (segment.GuideCurveExists(uid)) {
            return segment.GetGuideCurve(uid);
        }
    }
    throw tigl::CTiglError("Error: Guide Curve with UID " + uid + " does not exists", TIGL_ERROR);
}

} // end namespace tigl
