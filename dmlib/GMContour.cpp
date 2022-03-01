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
#include "GMContour.h"
#include "CoreEventInfo.h"

/* Constructor */
GMContour::GMContour(
        double PGA,
        double PGV,
        int MMI,
        std::string PGA_units,
        std::string PGV_units,
        std::string MMI_units,
        std::string lat_units,
        std::string lon_units,
        std::string type) : _PGA(PGA), _PGV(PGV), _MMI(MMI),
                PGAunits(PGA_units), PGVunits(PGV_units), MMIunits(MMI_units),
                latUnits(lat_units), lonUnits(lon_units), _type(type) {}
   
/* Destructor */ 
GMContour::~GMContour() {}

/* Sets the value for the PGA threshold for this GMContour object
 * @param PGA PGA threshold displayed by this contour
 * @return the PGA threshold for this contour */
double GMContour::setPGA(double PGA) { _PGA = PGA; return _PGA; }

/* Sets the value for the PGV threshold for this GMContour object
 * @param PGV PGV threshold displayed by this contour
 * @return the PGV threshold for this contour */
double GMContour::setPGV (double PGV) { _PGV = PGV; return _PGV; }

/* Sets the value for the MMI threshold for this GMContour object
 * @param MMI MMI threshold displayed by this contour
 * @return the MMI threshold for this contour */
int GMContour::setMMI (int MMI) { _MMI = MMI; return _MMI; }

/* Sets the units for MMI (if chosen as contour type) for this GMContour object
 * @param std::string containing units for MMI
 * @return std::string units in which MMI is provided */
std::string GMContour::setMMIunits (std::string MMI_units) { MMIunits = MMI_units; return MMIunits; }

/* Sets the units for PGV (if chosen as contour type) for this GMContour object
 * @param std::string containing units for PGV
 * @return std::string units in which PGV is provided */
std::string GMContour::setPGVunits (std::string PGV_units) { PGVunits = PGV_units; return PGVunits; }

/* Sets the units for PGA (if chosen as contour type) for this GMContour object
 * @param std::string containing units for PGA
 * @return std::string units in which PGA is provided */
std::string GMContour::setPGAunits (std::string PGA_units) { PGAunits = PGA_units; return PGAunits; }

/* Sets the units for longitude for this GMContour object
 * @param std::string containing units for longitude
 * @return std::string units in which longitude is provided */
std::string GMContour::setLatUnits (std::string lat_units) { latUnits = lat_units; return latUnits; }

/* Sets the units for latitude for this GMContour object
 * @param std::string containing units for latitude
 * @return std::string units in which latitude is provided */
std::string GMContour::setLonUnits (std::string lon_units) { lonUnits = lon_units; return lonUnits; }

/* Sets the chosen ground motion type for this GMContour object
 * @param std::string representing return type
 * @return std::string type for this contour: PGA, PGV, MMI */
std::string GMContour::setType (std::string type) { _type = type; return _type; }

/* Returns the chosen ground motion type for this GMContour object
 * @return std::string type for this contour: PGA, PGV, MMI */
std::string GMContour::getType () const { return _type; }

/* Returns the chosen PGA threshold for this GMContour object
 * @return double PGV threshold for this contour */
double GMContour::getPGA() const { return _PGA; }

/* Returns the chosen PGV threshold for this GMContour object
 * @return double PGV threshold for this contour */
double GMContour::getPGV () const { return _PGV; }

/* Returns the chosen MMI threshold for this GMContour object
 * @return double MMI threshold for this contour */
int GMContour::getMMI () const { return _MMI; }

/* Returns private member std::string representing units in which MMI is delivered
 * @return std::string containing MMI units */
std::string GMContour::getMMIunits () const { return MMIunits; }

/* Returns private member std::string representing units in which PGV is delivered
 * @return std::string containing PGV units */
std::string GMContour::getPGVunits () const { return PGVunits; }

/* Returns private member std::string representing units in which PGA is delivered
 * @return std::string containing PGA units */
std::string GMContour::getPGAunits () const { return PGAunits; }

/* Returns private member std::string representing units in which latitude is delivered
 * @return std::string containing latitude units */
std::string GMContour::getLatUnits () const { return latUnits; }

/* Returns private member std::string representing units in which longitude is delivered
 * @return std::string containing longitude units */
std::string GMContour::getLonUnits () const { return lonUnits; }

//gets and sets for lats vector and lons vector defined in .h file

/* Sets all members of calling GMContour object to members of argument GMContour object 
 * @param from reference to GMContour object that calling GMContour object is updating 
 *          its contents to match */
void GMContour::updateFrom(const GMContour &from)
{
    _PGA = from._PGA;
    _PGV = from._PGV;
    _MMI = from._MMI;
    PGAunits = from.PGAunits;
    PGVunits = from.PGVunits;
    MMIunits = from.MMIunits;
    latUnits = from.latUnits; 
    lonUnits = from.lonUnits;
    _type = from._type;
    _lats = from._lats;
    _lons = from._lons;
}

/* Checks if two GMContour objects are equal: if their contents match exactly 
 * @param A reference to a GMContour object
 * @return bool indicating equality (true) or inequality (false) */
bool GMContour::operator==(const GMContour &A) const
{
    return ( A._PGA == _PGA &&
	A._PGV == _PGV &&
	A._MMI == _MMI &&
	A.PGAunits == PGAunits &&
	A.PGVunits == PGVunits &&
	A.MMIunits == MMIunits &&
	A.latUnits == latUnits &&
	A.lonUnits == lonUnits &&
	A._type == _type &&
	A._lats == _lats &&
    A._lons == _lons);
}

/* Checks if two GMContour objects are not equal: if their contents do not match exactly 
 * @param A reference to a GMContour object
 * @return bool indicating inequality (true) or equality (false) */
bool GMContour::operator!=(const GMContour &A) const { return !(*this==A); }

/* Creates a string containing all contents of GMContour object */
string GMContour::toString()
{
    stringstream s;
    s << setiosflags(ios::fixed) << setprecision(4);

    s << " PGA: " << getPGA() << " " << getPGAunits() << std::endl;
    s << " PGV: " << getPGV() << " " << getPGVunits() << std::endl;
    s << " MMI: " << getMMI() << " " << getMMIunits() << std::endl;

    s << " Lat Units: " << getLatUnits() << std::endl;
    s << " Lon Units: " << getLonUnits() << std::endl;

    s << " Type: " << getType() << std::endl;
    
    s << "Vertices: ";
    for (size_t i = 0; i < _lats.size(); i++) {
        s << _lats.at(i) << ", " << _lons.at(i) << std::endl; 
    }
    
    return s.str();
}

/* Prints result of toString function to standard output */ 
void GMContour::coutPrint()
{
    cout << toString() << std::endl;
}
