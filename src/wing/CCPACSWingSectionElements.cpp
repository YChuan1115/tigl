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
* @brief  Implementation of CPACS wing section elements handling routines.
*/

#include "CCPACSWingSectionElements.h"
#include "CTiglError.h"

namespace tigl
{
CCPACSWingSectionElements::CCPACSWingSectionElements(CCPACSWingSection* parent, CTiglUIDManager* uidMgr)
    : generated::CPACSWingElements(parent, uidMgr) {}

// Get element count for this section
int CCPACSWingSectionElements::GetSectionElementCount() const
{
    return static_cast<int>(m_elements.size());
}

// Get element for a given index
CCPACSWingSectionElement& CCPACSWingSectionElements::GetSectionElement(int index) const
{
    index--;
    if (index < 0 || index >= GetSectionElementCount()) {
        throw CTiglError("Invalid index in CCPACSWingSectionElements::GetSectionElement", TIGL_INDEX_ERROR);
    }
    return *m_elements[index];
}


} // end namespace tigl
