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
#ifndef __GMContour_h
#define __GMContour_h

#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

/** A base class for contour ground motion information.
 *  @ingroup dm_lib
 */
class GMContour
{
 private:
    double _PGA;
    double _PGV;
    int _MMI;
    std::string PGAunits;
    std::string PGVunits;
    std::string MMIunits;
    std::string latUnits;
    std::string lonUnits;
    std::string _type;
    std::vector<double> _lats;
    std::vector<double> _lons;
    
 public:
    GMContour(
              double PGA = -999.9999, 
              double PGV = -999.9999, 
              int MMI = -999,
              std::string PGA_units = "cm/s/s",
              std::string PGV_units = "cm/s",
              std::string MMI_units = "",
              std::string lat_units = "deg",
              std::string lon_units = "deg",
              std::string type = "MMI");

    virtual ~GMContour();
    
    double setPGA(double PGA);
    double setPGV(double PGV);
    int setMMI(int MMI);

    std::string setPGAunits(std::string PGA_units);
    std::string setPGVunits(std::string PGV_units);
    std::string setMMIunits(std::string MMI_units);
    std::string setLatUnits(std::string lat_units);
    std::string setLonUnits(std::string lon_units);

    std::string setType(std::string type);

    /* Return lats describing a GMContour object
    @return vector<double> containing lat locations for vertices of contour polygon */
    std::vector<double> setLats(std::vector<double> lats) { _lats = lats; return _lats; }

    /* Return lons describing a GMContour object
    @return vector<double> containing lon locations for vertices of contour polygon */
    std::vector<double> setLons(std::vector<double> lons) { _lons = lons; return _lons; } 
 
    double getPGA() const;
    double getPGV()const;
    int getMMI()const;

    std::string getPGAunits() const;
    std::string getPGVunits()const;
    std::string getMMIunits()const;
    std::string getLatUnits()const;
    std::string getLonUnits()const;

    std::vector<double> getLats() const { return _lats; }
    std::vector<double> getLons() const { return _lons; }
    int getNumberContourVertices() {return _lats.size(); };

    std::string getType() const;

    virtual void updateFrom(const GMContour &); 

    virtual bool operator==(const GMContour &A) const;
    virtual bool operator!=(const GMContour &A) const;

    void coutPrint();
    std::string toString();
};

#endif
