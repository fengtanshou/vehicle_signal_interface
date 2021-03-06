/*
    Copyright (C) 2016-2017, Jaguar Land Rover. All Rights Reserved.

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

/*!----------------------------------------------------------------------------

    @file writeRecord.c

    This file will write a single record into the shared memory message buffers.

-----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <stdbool.h>

#include "vsi.h"
#include "vsi_core_api.h"
#include "signals.h"


/*! @{ */

//
//  Define the usage message function.
//
static void usage ( const char* executable )
{
    printf ( " \n\
Usage: %s options\n\
\n\
  Option     Meaning       Type     Default   \n\
  ======  ==============  ======  =========== \n\
    -a    ASCII Value     string      N/A     \n\
    -d    Domain Value     int         1      \n\
    -h    Help Message     N/A        N/A     \n\
    -s    Signal Value     int        N/A     \n\
    -v    Data Value      long    Same as key \n\
    -?    Help Message     N/A        N/A     \n\
\n\n\
",
    executable );
}


/*!-----------------------------------------------------------------------

    m a i n

    @brief The main entry point for this compilation unit.

    This function will insert a single message into the shared memory segment
    as specified by the user.

    The "domain" will default to "CAN" if not specified by the caller and the
    "signal" value will default to 0.  The body data (8 bytes) will default to
    the same thing as the key if not specified.

    Note that the data value will be interpreted as an 8 byte numeric value.

    @return  0 - This function completed without errors
    @return !0 - The error code that was encountered

------------------------------------------------------------------------*/
int main ( int argc, char* const argv[] )
{
    //
    //  The following locale settings will allow the use of the comma
    //  "thousands" separator format specifier to be used.  e.g. "10000"
    //  will print as "10,000" (using the %'u spec.).
    //
    setlocale ( LC_ALL, "");

    //
    //  Parse any command line options the user may have supplied.
    //
    unsigned long dataValue = 0;
    char*         dataString = NULL;
    int           dataLength = 8;
    signal_t      signal = 0;
    domain_t      domain = 1;
    char          ch;

    while ( ( ch = getopt ( argc, argv, "a:d:hs:v:?" ) ) != -1 )
    {
        switch ( ch )
        {
          //
          //  Get the requested ASCII string signal value.
          //
          case 'a':
            dataString = optarg;
            dataLength = strlen ( dataString );

            //
            //  If the string length is exactly 8 bytes, increment it to 9 so
            //  we can tell the difference between and 8 byte number and an 8
            //  byte string.  Note that this results in the null byte at the
            //  end of the string being stored in the shared memory segment
            //  but this should not hurt anything.
            //
            //  Because of this logic, there will never be an 8 byte ASCII
            //  string stored as a signal value.  When reading the signal
            //  value back out, we can check to see if it is 8 bytes in length
            //  and assume that if it is it is a numeric value.  Any other
            //  length value means that the value is an ASCII string.
            //
            if ( dataLength == 8 )
            {
                ++dataLength;
            }
            LOG ( "Data String value[%s]\n", dataString );
            break;

          //
          //  Get the requested domain value.
          //
          case 'd':
            domain = atol ( optarg );
            LOG ( "Using domain value[%'d]\n", domain );
            break;

          //
          //  Get the requested signal value.
          //
          case 's':
            signal = atoi ( optarg );
            LOG ( "Using signal value[%'d]\n", signal );
            break;

          //
          //  Get the data value from the user.
          //
          case 'v':
            dataValue = atol ( optarg );
            dataLength = sizeof(dataValue);
            LOG ( "Data value[%'lu] will be used.\n", dataValue );
            break;

          //
          //  Display the help message.
          //
          case 'h':
          case '?':
          default:
            usage ( argv[0] );
            exit ( 0 );
        }
    }
    //
    //  If the user supplied any arguments not parsed above, they are not
    //  valid arguments so complain and quit.
    //
    argc -= optind;
    if ( argc != 0 )
    {
        printf ( "Invalid parameters[s] encountered: %s\n", argv[argc] );
        usage ( argv[0] );
        exit (255);
    }
    //
    //  Open the shared memory file.
    //
    //  Note that if the shared memory segment does not already exist, this
    //  call will create it.
    //
    vsi_initialize ( false );

    //
    //  Go insert this message into the signal lists.
    //
    if ( dataLength == sizeof(dataValue ) )
    {
        vsi_core_insert ( domain, signal, dataLength, &dataValue );
    }
    else
    {
        vsi_core_insert ( domain, signal, dataLength, dataString );
    }
#ifdef VSI_DEBUG
    dumpSignals();
#endif

    //
    //  Close our shared memory segment and exit.
    //
    vsi_core_close();

    //
    //  Return a good completion code to the caller.
    //
    return 0;
}


/*! @} */

// vim:filetype=c:syntax=c
