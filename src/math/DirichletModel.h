/*
 * DirichletModel.h
 *
 *  Created on: Jun 16, 2016
 *      Author: zhengqi
 */

#ifndef SRC_MATH_DIRICHLETMODEL_H_
#define SRC_MATH_DIRICHLETMODEL_H_

#include <cassert>
#include <cfloat>
#include <iostream>
#include <eigen3/Eigen/Dense>

namespace EGriceLab {
namespace Math {

using std::istream;
using std::ostream;
using Eigen::VectorXd;
using Eigen::MatrixXd;

class DirichletModel {
public:
	/* default constructor disabled */
/*	DirichletModel(): K(MIN_K) { }*/

	/* construct a Dirichlet model with given categories */
	explicit DirichletModel(int K): K(K) {
		assert(K >= MIN_K);
	}

	/* virtual destructor, do nothing */
	virtual ~DirichletModel() { }

	/* member methods */
	/**
	 * An abstract method to calculate the posterior probabilities of category
	 * given the parameters and an observed frequency
	 */
	virtual VectorXd postP(const VectorXd& freq) const = 0;

	/**
	 * Do a maximum likelihood training of all underlying parameters given a training data,
	 * with M columns each an observed frequency vector, and K rows
	 */
	virtual void trainML(const MatrixXd& data,
			double eta = DEFAULT_ETA, double epsilonCost = DEFAULT_EPSILON_COST,
			double epsilonParams = DEFAULT_EPSILON_PARAMS, int maxIt = MAX_ITERATION) = 0;

	/**
	 * Calculate the negative gradient of the Dirichlet parameters
	 * using current parameters and observed data at (unbound) exponential scale
	 * Exponential scale is used because the Dirichlet parameters must be strictly positive.
	 */
	virtual VectorXd expGradient(const MatrixXd& data) const = 0;

	/**
	 * Calculate the logPDF of observing a data using this model
	 */
	virtual double lpdf(const VectorXd& freq) const = 0;

	/**
	 * Calculate the PDF of observing a data using this model
	 */
	virtual double pdf(const VectorXd& freq) const;

	/**
	 * Calculate the cost of observing an entire data
	 */
	double cost(const MatrixXd& data) const;

	/*
	 * internal methods to support input/output method inheritance
	 */
	virtual ostream& print(ostream& out) const = 0;
	virtual istream& read(istream& in) = 0;

public:
	/* non-member friend functions */
	friend ostream& operator<<(ostream& out, const DirichletModel& dm);
	friend istream& operator>>(istream& in, DirichletModel& dm);

	/* getters and setters */
	int getK() const {
		return K;
	}

private:
	int K; // number of parameters

public:
	static const double DEFAULT_ETA = 0.05; // default step width relative to the gradient used in ML parameter training
	static const int MIN_K = 2; // minimum number of categories
//	static const double DEFAULT_EPSILON = FLT_EPSILON;
	static const double DEFAULT_EPSILON_COST = 1e-10;
	static const double DEFAULT_EPSILON_PARAMS = 1e-4;
	static const int MAX_ITERATION = 0;
};

inline ostream& operator<<(ostream& out, DirichletModel& dm) {
	return dm.print(out);
}

inline istream& operator>>(istream& in, DirichletModel& dm) {
	return dm.read(in);
}

inline double DirichletModel::pdf(const VectorXd& data) const {
	return ::exp(lpdf(data));
}

inline double DirichletModel::cost(const MatrixXd& data) const {
	double c = 0;
	MatrixXd::Index M = data.cols();
	for(MatrixXd::Index t = 0; t < M; ++t) {
		c -= lpdf(data.col(t));
	}
	return c;
}

} /* namespace Math */
} /* namespace EGriceLab */

#endif /* SRC_MATH_DIRICHLETMODEL_H_ */
