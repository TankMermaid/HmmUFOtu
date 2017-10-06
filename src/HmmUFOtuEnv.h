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
 * HmmUFOtuEnv.h
 *  Per-application environment variables
 *  Created on: Nov 18, 2015
 *      Author: zhengqi
 */

#ifndef SRC_HMMUFOTUENV_H_
#define SRC_HMMUFOTUENV_H_
#include <string>
#include <iostream>

#include "VersionSequence.h"

namespace EGriceLab {
/* per-application variables */
extern int VERBOSE_LEVEL; /* DEFAULT VERBOSE LEVEL */
extern const VersionSequence progVer;

enum LOG_LEVEL {
	LOG_NOTHING,
	LOG_ERROR,
	LOG_WARNING,
	LOG_INFO,
	LOG_DEBUG
};


/**
 * show program and package version
 */
void printVersion(const string& app, ostream& out = std::cerr);

/**
 * get full program name
 */
inline string getProgFullName(const string& name, const VersionSequence& ver) {
	return name + "-" + ver.toString();
}

}

#endif /* SRC_HMMUFOTUENV_H_ */
