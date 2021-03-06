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
 * DNASubModel.cpp
 *
 *  Created on: Apr 1, 2016
 *      Author: zhengqi
 */

#include <vector>
#include <set>
#include <vector>
#include <stack>
#include "DNASubModel.h"

namespace EGriceLab {
namespace HmmUFOtu {

using namespace std;
using namespace Eigen;

const double DNASubModel::MAX_PDIST = 0.15; /* maximum p-dist between training sequences */
const IOFormat DNASubModel::FULL_FORMAT(Eigen::FullPrecision);
const IOFormat DNASubModel::STD_FORMAT(Eigen::StreamPrecision);

DigitalSeq::size_type DNASubModel::nonGapSites(const DigitalSeq& seq1, const DigitalSeq& seq2, int start, int end) {
	assert(seq1.length() == seq2.length());
	DigitalSeq::size_type N = 0;
	for(DigitalSeq::size_type i = start; i <= end; ++i)
		if(seq1.isSymbol(i) && seq2.isSymbol(i))
			N++;
	return N;
}

Matrix4d DNASubModel::calcTransFreq2Seq(const DigitalSeq& seq1, const DigitalSeq& seq2) {
	assert(seq1.getAbc() == seq2.getAbc());
	assert(seq1.length() == seq2.length());
	Matrix4d freq = Matrix4d::Zero();

	const DigitalSeq::size_type L = seq1.length();
	for(DigitalSeq::size_type i = 0; i < L; ++i)
		if(seq1.isSymbol(i) && seq2.isSymbol(i)) // both not a gap
			freq(seq1[i], seq2[i])++;
	return freq;
}

Matrix4d DNASubModel::calcObservedDiff(const DigitalSeq& seq1, const DigitalSeq& seq2, int start, int end) {
	assert(seq1.getAbc() == seq2.getAbc());
	assert(seq1.length() == seq2.length());
	Matrix4d freq = Matrix4d::Zero();

	for(DigitalSeq::size_type i = start; i <= end; ++i)
		if(seq1.isSymbol(i) && seq2.isSymbol(i)) // both not a gap
			freq(seq1[i], seq2[i])++;
	return freq;
}

Matrix4d DNASubModel::calcTransFreq3Seq(const DigitalSeq& outer,
		const DigitalSeq& seq1, const DigitalSeq& seq2) {
	assert(outer.getAbc() == seq1.getAbc() && outer.getAbc() == seq2.getAbc());
	assert(outer.length() == seq1.length() && outer.length() == seq2.length());
	Matrix4d freq = Matrix4d::Zero();

	const DigitalSeq::size_type L = outer.length();
	for(DigitalSeq::size_type i = 0; i < L; ++i) {
		int8_t b0 = outer[i];
		int8_t b1 = seq1[i];
		int8_t b2 = seq2[i];
		int8_t bc; // ancestor of b0, b1 and b2
		if(!(b0 >= 0 && b1 >= 0 &&  b2 >= 0)) // ignore any gaps
			continue;
		if(b0 == b1 && b0 == b2) // no change
			bc = b0;
		else if(b0 == b1 && b0 != b2) // change is between bc->b2
			bc = b0;
		else if(b0 == b2 && b0 != b1) // change is between bc->b1
			bc = b0;
		else if(b0 != b1 && b0 != b2 && b1 == b2) // change is between bc->b0
			bc = b1;
		else // all different, cannot guess
			continue;
		freq(bc, b0)++;
		freq(bc, b1)++;
		freq(bc, b2)++;
	}
	return freq;
}

Vector4d DNASubModel::calcBaseFreq(const DigitalSeq& seq) {
	Vector4d f = Vector4d::Zero();
	for(DigitalSeq::const_iterator it = seq.begin(); it != seq.end(); ++it)
		if(*it >= 0)
			f(*it)++;
	return f;
}

double DNASubModel::subDist(const DigitalSeq& seq1, const DigitalSeq& seq2, int start, int end) const {
	assert(seq1.getAbc() == seq2.getAbc());
	assert(seq1.length() == seq2.length());
	return subDist(
			calcObservedDiff(seq1, seq2, start, end),
			nonGapSites(seq1, seq2, start, end)
	);
}

Matrix4d DNASubModel::scale(Matrix4d Q, Vector4d pi, double mu) {
	double beta = pi.dot(Q.diagonal());
	return Q / -beta * mu;
}

Matrix4d DNASubModel::logQfromP(Matrix4d P, bool reversible) {
	if(reversible)
		P = (P + P.transpose()) / 2.0;
	/* normalize P */
	for(Matrix4d::Index i = 0; i < P.rows(); ++i)
		P.row(i) /= P.row(i).sum();

	/* do matrix log by diagonalizable matrix decomposition */
	EigenSolver<Matrix4d> es(P);
	if(es.info() != Eigen::Success) {
		cerr << "Cannot perform EigenSolver on observed frequency data P:" << endl << P << endl;
		abort();
	}
	Matrix4cd PSI = es.eigenvalues().asDiagonal(); /* eigen values of P */
	Matrix4cd U = es.eigenvectors();
	Matrix4cd U_1 = U.inverse();
	return (U * PSI.array().log().matrix() * U_1).real();
}

Matrix4d DNASubModel::constrainedQfromP(Matrix4d P, bool reversible) {
	if(reversible)
		P = (P + P.transpose()) / 2.0;
	Vector4d Z = P.rowwise().sum(); // normalizing constants
	Matrix4d Q = Matrix4d::Zero();
	/* set the elements */
	for(Matrix4d::Index i = 0; i < Q.rows(); ++i) {
		for(Matrix4d::Index j = 0; j < Q.cols(); ++j) {
			if(i != j) {
				Q(i, j) = P(i, j) / Z(i); /* non-diagonal */
				Q(i, i) -= Q(i, j);       /* diagonal */
			}
		}
	}
//	cerr << "P: " << P << endl;
//	cerr << "Q: " << Q << endl;
	return Q;
}

} /* namespace HmmUFOtu */
} /* namespace EGriceLab */
