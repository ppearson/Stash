#!/usr/bin/python

# Translate utility for managing localisation strings with xib resource bundles for Stash
# version 1.0

import sys
import os

xibFiles = ["AccountInfo", "AddAccount", "DueScheduledTransactions", "ExportOFX", "ImportOFX", "ImportQIF", "MainMenu", "MakeTransfer", "Preferences"]

def generateStringFiles(pathToCreateIn):
	if pathToCreateIn[-1] != "/":
		pathToCreateIn += "/"
	
	for file in xibFiles:
		strCommand = "ibtool --export-strings-file %s%s.strings en.lproj/%s.xib" % (pathToCreateIn, file, file)
#		print strCommand
		os.system(strCommand)
	
	print "String files generated."

def createXibFiles(pathOfStringFiles, newLocalisation):
	if pathOfStringFiles[-1] != "/":
		pathOfStringFiles += "/"
		
	strNewDir = "%s.lproj/" % (newLocalisation)
	
	try:
		os.mkdir(strNewDir)
	except os.error:
		pass
	
	for file in xibFiles:
		strCommand = "ibtool --strings-file %s%s.strings --write %s%s.xib en.lproj/%s.xib" % (pathOfStringFiles, file, strNewDir, file, file)
#		print strCommand
		os.system(strCommand)
	
	print ".xib files created."

if __name__ == '__main__':
	if len(sys.argv) < 2:
		print "Invalid parameters specified.\nCommands are:\ntranslate.py genstrings <pathForStringFiles>\ntranslate.py createxibs <pathOfStringFiles> <newLocalisationName>\n"
		sys.exit(2)
	
	if sys.argv[1] == "genstrings":
		pathToCreateIn = sys.argv[2]
		if len(pathToCreateIn) > 0:
			generateStringFiles(pathToCreateIn)
	elif sys.argv[1] == "createxibs":
		pathOfStringFiles = sys.argv[2]
		newLocalisation = sys.argv[3]
		createXibFiles(pathOfStringFiles, newLocalisation)

