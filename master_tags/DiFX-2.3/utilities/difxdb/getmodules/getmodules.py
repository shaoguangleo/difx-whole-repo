#! /usr/bin/python

#===========================================================================
# SVN properties (DO NOT CHANGE)
#
# $Id$
# $HeadURL$
# $LastChangedRevision$
# $Author$
# $LastChangedDate$
#
#============================================================================

import os
import sys
from difxdb.difxdbconfig import DifxDbConfig
from difxdb.model.dbConnection import Schema, Connection
from difxdb.business.experimentaction import *
from difxdb.model import model
from operator import  attrgetter

__author__="Helge Rottmann <rottmann@mpifr-bonn.mpg.de>"
__prog__ = os.path.basename(__file__)
__build__= "$Revision$"
__date__ ="$Date$"
__lastAuthor__="$Author$"

def printUsage():
    print "%s   %s  %s (last changes by %s) \n" % (__prog__, __build__, __author__, __lastAuthor__)
    print "A program to list all modules associated with an experiment\n"
    print "Usage: %s <experiment_code>\n\n"  % __prog__
    print "NOTE: %s requires the DIFXROOT environment to be defined." % __prog__
    print "The program reads the database configuration from difxdb.ini located under $DIFXROOT/conf."
    print "If the configuration is not found a sample one will be created for you."

    
    sys.exit(1)
    

if __name__ == "__main__":
    
    
    if (len(sys.argv) != 2):
        printUsage()
    
    expCode = sys.argv[1]
    try:
        if (os.getenv("DIFXROOT") == None):
            sys.exit("Error: DIFXROOT environment must be defined.")

        configPath = os.getenv("DIFXROOT") + "/conf/difxdb.ini"


        config = DifxDbConfig(configPath, create=True)

        # try to open the database connection
        connection = Connection()
        connection.type = config.get("Database", "type")
        connection.server = config.get("Database", "server")
        connection.port = config.get("Database", "port")
        connection.user = config.get("Database", "user")
        connection.password = config.get("Database", "password")
        connection.database = config.get("Database", "database")
        connection.echo = False

        dbConn = Schema(connection)
        session = dbConn.session()
        
	try:
		experiment = getExperimentByCode(session, expCode)
	except:
		raise Exception("Unknown experiment")
		sys.exit

	sum = 0
	count = 0
	if (experiment is not None):
                sortedModules = sorted(experiment.modules, key= attrgetter('stationCode'))
		for module in sortedModules:
			print module.vsn, module.slot.location, module.stationCode, module.capacity
			count += 1
			sum += module.capacity

		print "--------------------------------------------"
		print "Total capacity: ", sum, "GB", " on", count , "modules"
        
    
    except Exception as e:
       
        sys.exit(e)
    
   
    
