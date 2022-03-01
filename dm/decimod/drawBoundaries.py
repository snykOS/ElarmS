# Draw the boundaries from the CA_latlon, PNW_latlon and finder_latlon parameters in the sa.conf file
# python drawBoundaries.py --conf=sa.conf --lat=37.0 --lon=-122.0

import sys, getopt
import re
import numpy as np
import matplotlib.pyplot as plt

def read_bndry(config, param, lat, lon):
    f = open(config, 'r')
    lines = f.readlines()
    for line in lines:
        if line.startswith(param):
            coords = line[len(param):]
            latlon = re.sub(r'\s', '', coords).split(',')
            for i in range(0,len(latlon),2):
                lat.append(latlon[i]);
                lon.append(latlon[i+1]);


def draw_boundaries(config, ofile, point_lat, point_lon):

    x = []
    y = []
    read_bndry(config, "CA_latlon", y, x)

    plt.plot(x, y, 'g')

    x = []
    y = []
    read_bndry(config, "PNW_latlon", y, x)

    plt.plot(x, y, 'b')

    x = []
    y = []
    read_bndry(config, "finder_latlon", y, x)

    if len(point_lat) > 0 and len(point_lon) > 0:
        plt.plot(point_lon, point_lat, 'bo')

    plt.plot(x, y, 'r')

    plt.show()


def main(argv):
    conf = "sa.conf"
    evid = 0
    ofile= ""
    lat = ""
    lon = ""

    try:
        opts, args = getopt.getopt(argv,"hcxy:o:",["conf=","lon=","lat=","ofile="])
    except getopt.GetoptError:
        print("drawBoundaries.py -c <conf> -o <ofile> [lat lon]")
        sys.exit(2);
    for opt, arg in opts:
        if opt == '-h':
            print("drawBoundaries.py -c <config> -o <ofile> [lat lon]")
            sys.exit()
        elif opt in ("-c", "--conf"):
            conf = arg
        elif opt in ("-x", "--lon"):
            lon = arg
        elif opt in ("-y", "--lat"):
            lat = arg
        elif opt in ("-o", "--ofile"):
            ofile = arg

    print("lat: " + lat)
    print("lon: " + lon)

    draw_boundaries(conf, ofile, lat, lon)

if __name__ == "__main__":
    main(sys.argv[1:])
