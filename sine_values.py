import sys
import math as m
num_values = 4096

def main(argv):
    print "const unsigned char dac_high[" + str(num_values) + "] ="
    print "{"
    for i in range(num_values):
        print "    " + str(int(round(m.sin( i * 2 * m.pi / num_values) * 2 + 2))) +\
                ("," if i != num_values - 1 else "")
    print "};"

    print "const unsigned char dac_low[" + str(num_values) + "] ="
    print "{"
    for i in range(num_values):
        print "    " + str(int(round(m.sin( i * 2 * m.pi / num_values) * 2**11 + 2**11)) & 0x00FF ) +\
                ("," if i != num_values - 1 else "")
    print "};"


if __name__ == '__main__':
    main(sys.argv)
