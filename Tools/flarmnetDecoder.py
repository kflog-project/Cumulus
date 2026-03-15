# decode FlarmNet data, used by XCSoar

if __name__ == "__main__":

    start = True
    with open("data.fln") as fh:
        for l in fh:
            if( start is True ) :
                start = False
                print( "%s" % l.upper() )
            else:
                ba = bytes.fromhex( l )
                #print( ba )
                print ( ba.decode('utf-8', errors='replace') )
    