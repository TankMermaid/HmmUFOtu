#include <iostream>
#include <fstream>
#include "CSFMIndex.h"

using namespace std;
using namespace EGriceLab;
int main(int argc, char *argv[]) {
	if(argc != 4) {
		cerr << "Usage:  " << argv[0] << " MSA-INFILE format CS-OUTFILE" << endl;
		return -1;
	}

	ofstream out(argv[3]);
	if(!out.is_open()) {
		cerr << "Unable to open " << argv[3] << endl;
		return -1;
	}

	MSA* msa = MSA::loadMSAFile("dna", argv[1], argv[2]);

	cerr << "msa loaded" << endl;

	CSFMIndex* fmIdx = CSFMIndex::build(msa);

	cerr << "FM-index built" << endl;

	if(!fmIdx->save(out)) {
		cerr << "Unable to save CS-FMindex database" << endl;
		return -1;
	}

	return 0;
}