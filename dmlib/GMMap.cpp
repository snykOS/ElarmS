/*****************************************************************************

    Copyright ©2017. 
    The Regents of the University of California (Regents). 
    Authors: Berkeley Seismological Laboratory. All Rights Reserved. 
    Permission to use, copy, modify, and distribute this software
    and its documentation for educational, research, and not-for-profit
    purposes, without fee and without a signed licensing agreement, is hereby
    granted, provided that the above copyright notice, this paragraph and the
    following four paragraphs appear in all copies, modifications, and
    distributions. Contact The Office of Technology Licensing, UC Berkeley,
    2150 Shattuck Avenue, Suite 510, Berkeley, CA 94720-1620, (510) 643-7201,
    otl@berkeley.edu, http://ipira.berkeley.edu/industry-info for commercial
    licensing opportunities.

    Neither the name of the copyright holder nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

    Attribution Rights. You must retain, in the Source Code of any Derivative
    Works that You create, all copyright, patent, or trademark notices from
    the Source Code of the Original Work, as well as any notices of licensing
    and any descriptive text identified therein as an "Attribution Notice."
    You must cause the Source Code for any Derivative Works that You create to
    carry a prominent Attribution Notice reasonably calculated to inform
    recipients that You have modified the Original Work.

    IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
    SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
    ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
    REGENTS HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
    PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
    HEREUNDER IS PROVIDED "AS IS". REGENTS HAS NO OBLIGATION TO PROVIDE
    MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

*****************************************************************************/
#include <iostream>
#include <iomanip>
#include "GMMap.h"
#include "CoreEventInfo.h"

/* Constructor */
GMMap::GMMap(
    std::string PGA_units,
    std::string PGV_units,
    std::string MMI_units,
    std::string lat_units,
    std::string lon_units,
    int nGridPts): PGAunits(PGA_units), PGVunits(PGV_units), MMIunits(MMI_units), 
            latUnits(lat_units), lonUnits(lon_units), 
            lonIndex("1"), latIndex("2"), pgaIndex("3"), pgvIndex("4"), mmiIndex("5"), 
            numGridPts(nGridPts) {}
   
/* Destructor */ 
GMMap::~GMMap() {}

/* Sets GMMap internal member variable for mapData to argument mapData
 * @param _mapData vector<gminfo::GM_INFO> containing all ground motion info across 
 *          west coast map grid 
 * @return the vector<gminfo::GM_INFO> containing all ground motion info across 
 *          west coast map grid */
std::vector<gminfo::GM_INFO> GMMap::setMapData (std::vector<gminfo::GM_INFO> _mapData) {
    mapData = _mapData;
    numGridPts = mapData.size();
    return mapData;
} 

/* Sets the units for MMI for this GMMap object
 * @param std::string containing units for MMI
 * @return std::string units in which MMI is provided */
std::string GMMap::setMMIunits (std::string MMI_units) { MMIunits = MMI_units; return MMIunits; }

/* Sets the units for PGV for this GMMap object
 * @param std::string containing units for PGV
 * @return std::string units in which PGV is provided */
std::string GMMap::setPGVunits (std::string PGV_units) { PGVunits = PGV_units; return PGVunits; }

/* Sets the units for PGA for this GMMap object
 * @param std::string containing units for PGA
 * @return std::string units in which PGA is provided */
std::string GMMap::setPGAunits (std::string PGA_units) { PGAunits = PGA_units; return PGAunits; }

/* Sets the units for latitude for this GMMap object
 * @param std::string containing units for latitude
 * @return std::string units in which latitude is provided */
std::string GMMap::setLatUnits (std::string lat_units) { latUnits = lat_units; return latUnits; }

/* Sets the units for longitude for this GMMap object
 * @param std::string containing units for longitude
 * @return std::string units in which longitude is provided */
std::string GMMap::setLonUnits (std::string lon_units) { lonUnits = lon_units; return lonUnits; }

/* Returns private member std::string representing units in which MMI is delivered
 * @return std::string containing MMI units */
std::string GMMap::getMMIunits () const { return MMIunits; }

/* Returns private member std::string representing units in which PGV is delivered
 * @return std::string containing PGV units */
std::string GMMap::getPGVunits () const { return PGVunits; }

/* Returns private member std::string representing units in which PGV is delivered
 * @return std::string containing PGV units */
std::string GMMap::getPGAunits () const { return PGAunits; }

/* Returns private member std::string representing units in which latitude is delivered
 * @return std::string containing latitude units */
std::string GMMap::getLatUnits () const { return latUnits; }

/* Returns private member std::string representing units in which longitude is delivered
 * @return std::string containing longitude units */
std::string GMMap::getLonUnits () const { return lonUnits; }

/* Returns the number of grid points in the ground motion info map
 * @return number of grid points in the ground motion info map across the west coast*/
int GMMap::getNumGridPts() const { return numGridPts; } 

/* Returns the index of the lat field for the GMMap object's message format
 * @return std::string index of lat field in ShakeMap grid format */
std::string GMMap::getLatIndex () const { return latIndex; }

/* Returns the index of the lon field for the GMMap object's message format
 * @return std::string index of lon field in ShakeMap grid format */
std::string GMMap::getLonIndex () const { return lonIndex; }

/* Returns the index of the PGA field for the GMMap object's message format
 * @return std::string index of PGA field in ShakeMap grid format */
std::string GMMap::getPGAIndex () const { return pgaIndex; }

/* Returns the index of the PGV field for the GMMap object's message format
 * @return std::string index of PGV field in ShakeMap grid format */
std::string GMMap::getPGVIndex () const { return pgvIndex; }

/* Returns the index of the MMI field for the GMMap object's message format
 * @return std::string index of MMI field in ShakeMap grid format */
std::string GMMap::getMMIIndex () const { return mmiIndex; }

//getMapData defined in .h file

/* Sets all members of calling GMMap object to members of argument GMMap object 
 * @param from reference to GMMap object that calling GMMap object is updating 
 *          its contents to match */
void GMMap::updateFrom(const GMMap &from)
{
    mapData = from.mapData;
    PGAunits = from.PGAunits;
    PGVunits = from.PGVunits;
    MMIunits = from.MMIunits;
    latUnits = from.latUnits;
    lonUnits = from.lonUnits;
    latIndex = from.latIndex;
    lonIndex = from.lonIndex;
    pgaIndex = from.pgaIndex;
    pgvIndex = from.pgvIndex;
    mmiIndex = from.mmiIndex;
    numGridPts = from.numGridPts;
}

/* Checks if two GMMap objects are equal: if their contents match exactly 
 * @param A reference to a GMMap object
 * @return bool indicating equality (true) or inequality (false) */
bool GMMap::operator==(const GMMap &A) const
{
    return ( isEqual(A.mapData, mapData) &&
	A.PGAunits == PGAunits &&
	A.PGVunits == PGVunits &&
	A.MMIunits == MMIunits &&
	A.latUnits == latUnits &&
	A.lonUnits == lonUnits &&
	A.latIndex == latIndex &&
	A.lonIndex == lonIndex &&
	A.pgaIndex == pgaIndex &&
	A.pgvIndex == pgvIndex &&
    A.mmiIndex == mmiIndex &&
	A.numGridPts == numGridPts);
}

/* Checks if two internal mapData (vector<gminfo::GM_INFO>) objects are equal:
 * if their contents match exactly 
 * @param mapData1 first GMMap object
 * @param mapData2 second GMMap object
 * @return bool indicating equality (true) or inequality (false) */
bool GMMap::isEqual(std::vector<gminfo::GM_INFO> mapData1, std::vector<gminfo::GM_INFO> mapData2) const {

    if (mapData1.size() != mapData2.size()) {
        return false;
    }
    for (size_t n = 0; n < mapData1.size(); n++) {
        if (!(mapData1.at(n).PGA == mapData2.at(n).PGA && 
                mapData1.at(n).PGV == mapData2.at(n).PGV && 
                mapData1.at(n).SI == mapData2.at(n).SI &&
                mapData1.at(n).PGASig == mapData2.at(n).PGASig &&
                mapData1.at(n).PGVSig == mapData2.at(n).PGVSig &&
                mapData1.at(n).latGM == mapData2.at(n).latGM &&
                mapData1.at(n).lonGM == mapData2.at(n).lonGM)) {
            return false;
        }
    }
    return true;
}    


/* Checks if two GMMap objects are not equal: if their contents do not match exactly 
 * @param A reference to a GMMap object
 * @return bool indicating inequality (true) or equality (false) */
bool GMMap::operator!=(const GMMap &A) const { return !(*this==A); }

/* Creates a string containing all contents of GMMap object */
string GMMap::toString()
{
    stringstream s;
    s << setiosflags(ios::fixed) << setprecision(4);

    s << "NumGridPoints: " << getNumGridPts() << std::endl;
   
    for (int i = 0; i < getNumGridPts(); i++) { 
        s << i << ": PGA: " << mapData.at(i).PGA << " " << getPGAunits() << "; " << 
            "PGV: " << mapData.at(i).PGV << " " << getPGVunits() << "; " <<
            "MMI: " << mapData.at(i).SI << " " << getMMIunits() << "; " << 
            "Lat: " << mapData.at(i).latGM << " " << getLatUnits() << "; " << 
            "Lon: " << mapData.at(i).lonGM << " " << getLonUnits() << std::endl;
    }

    s << "LatIndex: " << getLatIndex() << std::endl;
    s << "LonIndex: " << getLonIndex() << std::endl;
    s << "PGAIndex: " << getPGAIndex() << std::endl;
    s << "PGVIndex: " << getPGVIndex() << std::endl;
    s << "MMIIndex: " << getMMIIndex() << std::endl;

    return s.str();
}

/* Prints result of toString function to standard output */ 
void GMMap::coutPrint()
{
    cout << toString() << std::endl;
}
