// Copyright (c) 2016 RISC Software GmbH
//
// This file was generated by CPACSGen from CPACS XML Schema (c) German Aerospace Center (DLR/SC).
// Do not edit, all changes are lost when files are re-generated.
//
// Licensed under the Apache License, Version 2.0 (the "License")
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <string>
#include <tixi.h>
#include <vector>
#include "tigl_internal.h"
#include "UniquePtr.h"

namespace tigl
{
    class CTiglUIDManager;
    class CCPACSAircraftModel;
    
    namespace generated
    {
        // This class is used in:
        // CPACSVehicles
        
        // generated from /xsd:schema/xsd:complexType[54]
        class CPACSAircraft
        {
        public:
            TIGL_EXPORT CPACSAircraft(CTiglUIDManager* uidMgr);
            TIGL_EXPORT virtual ~CPACSAircraft();
            
            TIGL_EXPORT CTiglUIDManager& GetUIDManager();
            TIGL_EXPORT const CTiglUIDManager& GetUIDManager() const;
            
            TIGL_EXPORT virtual void ReadCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath);
            TIGL_EXPORT virtual void WriteCPACS(const TixiDocumentHandle& tixiHandle, const std::string& xpath) const;
            
            TIGL_EXPORT virtual const std::vector<unique_ptr<CCPACSAircraftModel> >& GetModels() const;
            TIGL_EXPORT virtual std::vector<unique_ptr<CCPACSAircraftModel> >& GetModels();
            
            TIGL_EXPORT virtual CCPACSAircraftModel& AddModel();
            TIGL_EXPORT virtual void RemoveModel(CCPACSAircraftModel& ref);
            
        protected:
            CTiglUIDManager* m_uidMgr;
            
            std::vector<unique_ptr<CCPACSAircraftModel> > m_models;
            
        private:
            #ifdef HAVE_CPP11
            CPACSAircraft(const CPACSAircraft&) = delete;
            CPACSAircraft& operator=(const CPACSAircraft&) = delete;
            
            CPACSAircraft(CPACSAircraft&&) = delete;
            CPACSAircraft& operator=(CPACSAircraft&&) = delete;
            #else
            CPACSAircraft(const CPACSAircraft&);
            CPACSAircraft& operator=(const CPACSAircraft&);
            #endif
        };
    }
    
    // Aliases in tigl namespace
    #ifdef HAVE_CPP11
    using CCPACSAircraft = generated::CPACSAircraft;
    #else
    typedef generated::CPACSAircraft CCPACSAircraft;
    #endif
}
