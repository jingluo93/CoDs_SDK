/**
 *  @file    CoDs.h
 *  @author  Sina Mirrazavi (sina.mirrazavi@epfl.ch)
 *  @date    2017
 *  @version 1.0
 *
 *  @brief Smooth transitions  from non/contact to contact environment
 *
 *
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include "eigen3/Eigen/Dense"
#include <math.h>
#include "ros/ros.h"

using namespace std;
using namespace Eigen;

const double epsilon=10e-20;


class CoDs
{
public:
	void initialize(int Dimen_state,double delta_dx,double F_d,double Gammma_free_motion,bool defined_surface);
	void Set_Gamma(double Gamma,VectorXd Normal,VectorXd q2,VectorXd q3);
	void Set_State(VectorXd State,VectorXd DState,VectorXd Original_Dynamic);
	void Set_Mass_Matrix(MatrixXd M);


	MatrixXd Calculate_Modulation();

	VectorXd Get_Normal_Velocity();

private:

	void Error();
	inline	bool everythingisreceived();
	inline	void everyfalse();

	bool Surface_;
	bool State_of_system_is_set_;
	bool State_of_surface_is_set_;


	int Dimen_state_;


	double Gamma_Value_;
	double Gammma_Threshold_;

	double delta_dx_;
	double F_d_;

	double NF_;
	double F_dNMN_;


	VectorXd N_;
	VectorXd q2_;
	VectorXd q3_;

	VectorXd X_;
	VectorXd DX_;
	VectorXd F_;

	MatrixXd Q_;
	MatrixXd Q_inv_;
	MatrixXd Lambda_;
	MatrixXd M_;


	MatrixXd InvMass_;


	VectorXd Normal_velocity_robot_;




};


class Gamma
{
public:



private:

	void Error();

};



