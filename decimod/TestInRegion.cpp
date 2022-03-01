#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>   
#include <vector>

using namespace std;

class Coord {
  public:
    double lat;
    double lon;

    Coord(double latitude, double longitude) {
        lat = latitude;
        lon = longitude;
    }
};

class Polygon {
  public:
    vector<Coord> polygon;
    vector<double> slope;

    Polygon(double *c, int num) {
        for(int i = 0; i < num; i += 2) {
            polygon.push_back(Coord(c[i], c[i+1]));
        }
        int npoly = polygon.size();
        for(int i = 0; i < npoly-1; i++) {
            double d = (polygon[i+1].lat != polygon[i].lat) ?
                        (polygon[i+1].lon - polygon[i].lon)/(polygon[i+1].lat - polygon[i].lat) : 0.;
            slope.push_back(d);
        }
    }

    bool pointInside(double lat, double lon) {
        bool in = false;
        int npoly = (int)polygon.size();
        for(int i = 0; i < npoly-1; i++) {
            if( ((polygon[i].lat > lat) != (polygon[i+1].lat > lat)) &&
                (lon < slope[i]*(lat - polygon[i].lat) + polygon[i].lon) )
            {
                in = !in;
            }
        }
        return in;
    }
};


static bool getDouble(const char *c, double *d)
{
    char *endptr, last_char;
    int n;

    if(*c != '\0') {
        *d = strtod(c, &endptr);
        last_char = *endptr;
        if(last_char == '\0') {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    double CA_latlon[] = {37.43,-117.76,39,-120,42,-120,42,-125,42,-126,40,-126,34.5,-121.25,31.5,-119.26405,31.5,-113.8930,34.5,-114,37.43,-117.76};
    double PNW_latlon[] = {42.0,-122.70000,42.0,-121.41670,42.0,-120.00000,42.0,-117.00000,49.0,-117.00000,50.0,-117.00000,50.0,-128.10000,47.99591,
                        -128.10000,47.99591,-126.77735,45.35223,-125.93440,43.0,-126.00000,42.0,-126.00000,42.0,-125.16666,42.0,-122.70000};
    double finder_latlon[] = {34.5,-120.8, 33.4,-118.0, 32.2,-117.2, 32.2,-115.0, 34.5,-114.0, 37.43,-117.76, 39.0,-120.0, 42.0,-120.0, 43.5,-118.0,
                        46.8,-118.0, 49.0,-122.0, 49.0,-123.3221, 48.5,-124.7, 45.0,-124.2, 42.8,-124.7, 40.2,-124.7, 34.5,-120.8};

    Polygon CA = Polygon(CA_latlon, 22);
    Polygon PNW = Polygon(PNW_latlon, 28);
    Polygon Finder = Polygon(finder_latlon, 34);

    if(argc < 3) {
        cout << "Usage: " << argv[0] << " lat lon" << endl;
        exit(1);
    }

    double lat, lon;

    if(!getDouble(argv[1], &lat)) {
        cout << "Invalid lat " << argv[1] << endl;
        cout << "Usage: " << argv[0] << " lat lon" << endl;
        exit(1);
    }
    if(!getDouble(argv[2], &lon)) {
        cout << "Invalid lon " << argv[2] << endl;
        cout << "Usage: " << argv[0] << " lat lon" << endl;
        exit(1);
    }

    cout << setiosflags(ios::fixed);

    string ca = CA.pointInside(lat, lon) ? "inside" : "outside";
    string pnw = PNW.pointInside(lat, lon) ? "inside" : "outside";
    string finder = Finder.pointInside(lat, lon) ? "inside" : "outside";

    cout << "lat: " << lat << " lon: " << lon << endl;
    cout << "CA boundary:     point is " << ca << endl;
    cout << "PNW boundary:    point is " << pnw << endl;
    cout << "Finder boundary: point is " << finder << endl;
}
