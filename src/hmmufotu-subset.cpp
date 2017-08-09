/*******************************************************************************
 * This file is part of HmmUFOtu, an HMM and Phylogenetic placement
 * based tool for Ultra-fast taxonomy assignment and OTU organization
 * of microbiome sequencing data with species level accuracy.
 * Copyright (C) 2017  Qi Zheng
 *
 * HmmUFOtu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HmmUFOtu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AlignerBoost.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * hmmufotu-subset.cpp
 *  subset OTUTable
 *  Created on: Aug 5, 2017
 *      Author: zhengqi
 */

#include <iostream>
#include <fstream>
#include <cctype>
#include <cfloat>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cerrno>
#include "HmmUFOtu_common.h"
#include "HmmUFOtu_OTU.h"

using namespace std;
using namespace EGriceLab;
using namespace Eigen;

/* default values */
static const string TABLE_FORMAT = "table";
static const unsigned long DEFAULT_SIZE = 0;
static const string DEFAULT_METHOD = "uniform";

/**
 * Print introduction of this program
 */
void printIntro(void) {
	cerr << "Subset (subsample) an OTUTable and prune the samples and OTUs if necessary" << endl;
}

/**
 * Print the usage information
 */
void printUsage(const string& progName) {
	cerr << "Usage:    " << progName << "  <OTU-IN> <OTU-OUT> <-s SIZE> [options]" << endl
		 << "OTU-IN   FILE                  : OTUTable generate from hmmufotu-sum" << endl
		 << "OTU-OUT  FILE                  : update OTUTable output" << endl
		 << "-s              LONG           : subset sample size for each sample" << endl
		 << "Options:    --method  STR      : subsetting method, either 'uniform' (wo/ replacement) or 'multinomial' (w/ placement) [" << DEFAULT_METHOD << "]" << endl
		 << "            -S|--seed  INT     : random seed used for subsetting, for debug purpose" << endl
		 << "            -v  FLAG           : enable verbose information, you may set multiple -v for more details" << endl
		 << "            -h|--help          : print this message and exit" << endl;
}

int main(int argc, char* argv[]) {
	/* variable declarations */
	string inFn;
	string outFn;
	ifstream in;
	ofstream out;

	unsigned long size = DEFAULT_SIZE;
	string method = DEFAULT_METHOD;
	unsigned seed = time(NULL); // using time as default seed

	/* parse options */
	CommandOptions cmdOpts(argc, argv);
	if(cmdOpts.hasOpt("-h") || cmdOpts.hasOpt("--help")) {
		printIntro();
		printUsage(argv[0]);
		return EXIT_SUCCESS;
	}

	if(cmdOpts.numMainOpts() != 2) {
		cerr << "Error:" << endl;
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}
	inFn = cmdOpts.getMainOpt(0);
	outFn = cmdOpts.getMainOpt(1);

	if(!cmdOpts.hasOpt("-s")) {
		cerr <<"Error: option -s is required"<< endl;
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}
	else
		size = ::atol(cmdOpts.getOptStr("-s"));

	if(cmdOpts.hasOpt("--method"))
		method = cmdOpts.getOpt("--method");

	if(cmdOpts.hasOpt("-S"))
		seed = ::atoi(cmdOpts.getOptStr("-S"));
	if(cmdOpts.hasOpt("--seed"))
		seed = ::atoi(cmdOpts.getOptStr("--seed"));

	if(cmdOpts.hasOpt("-v"))
		INCREASE_LEVEL(cmdOpts.getOpt("-v").length());

	/* validate options */
	if(size <= 0) {
		cerr << "-s must be positive integer" << endl;
		return EXIT_FAILURE;
	}

	/* open inputs */
	in.open(inFn.c_str());
	if(!in) {
		cerr << "Unable to open PTUTable '" << inFn << "': " << ::strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	/* open outputs */
	out.open(outFn.c_str());
	if(!out.is_open()) {
		cerr << "Unable to write to '" << outFn << "': " << ::strerror(errno) << endl;
		return EXIT_FAILURE;
	}

	/* load input OTUTable */
	infoLog << "Loading OTUTable" << endl;
	OTUTable otuTable;
	otuTable.load(in, TABLE_FORMAT);

	infoLog << "Subsetting OTUTable" << endl;
	otuTable.seed(seed);
	otuTable.subset(size, method);

	/* prune OTUTable */
	otuTable.pruneSamples(size);
	otuTable.pruneOTUs();

	/* write the OTU table */
	infoLog << "Writing OTUTable" << endl;
	out << otuTable;
}