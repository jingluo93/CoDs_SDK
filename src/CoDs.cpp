/**
 *  @file    CoDs.cpp
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


#include "CoDs.h"


/**
 *   @brief  Initializing the CoDs package.
 *
 *   @param  Dimen_state is the dimension of the state. It should be 3!
 *   @param  delta_dx is the desired velocity of the robot at the impact point.
 *   @param  F_d is the desired force of the robot at the contact phase.
 *   @param  Gammma_free_motion is the threshold which defines boundary between the free motion and the transition phases.
 *   @param  defined_surface is a boolean. If it is true, the package uses a planner surface defined in  the Gamma class. Otherwise it uses the user defined Gamma function.
 *   @return void
 */
void CoDs::initialize(int Dimen_state,double delta_dx,double F_d,double Gammma_free_motion, bool defined_surface)
{

	if (Dimen_state==3)
	{

	}
	else
	{
		cout<<"The dimension of the system should be 3; i.e. x y z!"<<endl;
		cout<<"Dimen_state "<<Dimen_state<<endl;
		Error();
	}
	if (0<=delta_dx)
	{
		cout<<"delta_dx should be negative! "<<delta_dx<<endl;
		Error();
	}
	if (F_d<0)
	{
		cout<<"F_d should be positive! "<<F_d<<endl;
		Error();
	}
	Dimen_state_=Dimen_state;
	Surface_=defined_surface;
	Gammma_Threshold_=Gammma_free_motion;

	delta_dx_=delta_dx;
	F_d_=F_d;



	N_.resize(Dimen_state_);	q2_.resize(Dimen_state_);	q3_.resize(Dimen_state_);
	X_.resize(Dimen_state_);	DX_.resize(Dimen_state_);	F_.resize(Dimen_state_);



	Q_.resize(Dimen_state_,Dimen_state_); 			Q_.setZero();
	Q_inv_.resize(Dimen_state_,Dimen_state_); 		Q_inv_.setZero();
	Lambda_.resize(Dimen_state_,Dimen_state_); 		Lambda_.setZero();
	M_.resize(Dimen_state_,Dimen_state_); 			M_.setZero();
	InvMass_.resize(Dimen_state_,Dimen_state_); 	InvMass_.setIdentity();


	Normal_velocity_robot_.resize(Dimen_state_); Normal_velocity_robot_.setZero();


	everyfalse();

}

/**
 *   @brief Setting the value of Gamma and the normal to the surface
 *
 *   @param  Gamma is the surface; i.e. 1<=Gamma is the free motion phase 0<Gamma<1 the transition phase and Gamma=<0 the contact phase.
 *   @param  Normal is the current normal vector to the surface.
 *   @param  q2 is the tangential vector to N;
 *   @param  q3 is the tangential vector to N and q3;
 *   @return void
 */
void CoDs::Set_Gamma(double Gamma,VectorXd Normal,VectorXd q2,VectorXd q3)
{

	if ((Normal.rows()==Dimen_state_)&&(q2.rows()==Dimen_state_)&&(q3.rows()==Dimen_state_)&&((N_.transpose()*q2_+N_.transpose()*q3_+q2.transpose()*q3_)(0,0)<0.0001))
	{
	}
	else
	{
		cout<<"Something is wrong in Set_Gamma"<<endl;
		cout<<"Dimension of states is: "<<Dimen_state_<<endl;
		cout<<"Normal "<<Normal.cols()<<endl;cout<<Normal<<endl;
		cout<<"q2 "<<q2.cols()<<endl;cout<<q2<<endl;
		cout<<"q3 "<<q3.cols()<<endl;cout<<q3<<endl;
		cout<<"(N_.transpose()*q2_+N_.transpose()*q3_+q2.transpose()*q3_)(0,0) "<<(N_.transpose()*q2_+N_.transpose()*q3_+q2.transpose()*q3_)(0,0)<<endl;
		Error();
	}



	Gamma_Value_=Gamma;
	N_=Normal;
	q2_=q2;
	q3_=q3;

	Q_.col(0)=N_;	Q_.col(1)=q2_;	Q_.col(2)=q3_;

	Q_inv_=Q_.inverse();
	State_of_surface_is_set_=true;
}

/**
 *   @brief Setting the state of the system and the original dynamic
 *
 *   @param  State is the state of the system.
 *   @param  Original_Dynamic is the original dynamic.
 *   @return void
 */

void CoDs::Set_State(VectorXd State,VectorXd DState,VectorXd Original_Dynamic)
{

	if ((State.rows()==Dimen_state_)&&(Original_Dynamic.rows()==Dimen_state_)&&(DState.rows()==Dimen_state_))
	{
	}
	else
	{
		cout<<"Something is wrong in Set_State"<<endl;
		cout<<"Dimension of states is: "<<Dimen_state_<<endl;
		cout<<"State "<<State<<endl;
		cout<<"Original_Dynamic "<<Original_Dynamic<<endl;
		Error();
	}

	X_=State;
	DX_=DState;
	F_=Original_Dynamic;
	State_of_system_is_set_=true;
}

/**
 *   @brief Setting the mass matrix of the robot, it should be in the Cartesian space and it should be 3$\times$3
 *
 *   @param  State is the states of the system.
 *   @return void
 */
void CoDs::Set_Mass_Matrix(MatrixXd M)
{

	if ((M.cols()==Dimen_state_)&&(M.rows()==Dimen_state_))
	{
	}
	else
	{
		cout<<"Something is wrong in Set_Mass_Matrix"<<endl;
		cout<<"Mass matrix "<<M<<endl;
		Error();
	}

	InvMass_=M.inverse();
}

/**
 *   @brief Calculate the modulation function
 *
 *   @return The modulation function
 */
MatrixXd CoDs::Calculate_Modulation()
{

	if (!everythingisreceived())
	{
		cout<<"Even though you forgot setting sth, you called Calculate_Modulation"<<endl;
		cout<<"State_of_system_is_set_ "<<State_of_system_is_set_<<endl;
		cout<<"State_of_surface_is_set_ "<<State_of_surface_is_set_<<endl;
		Error();
	}

	Lambda_.setIdentity();

	if (Gammma_Threshold_<=Gamma_Value_)
	{
		Lambda_(0,0)=(1-exp(-(Gamma_Value_-Gammma_Threshold_)));
		Lambda_(1,1)=Lambda_(0,0);
		Lambda_(2,2)=Lambda_(0,0);
		cout<<"Free Motion"<<endl;
	}
	if((Gamma_Value_<Gammma_Threshold_)&&(0<Gamma_Value_))
	{
		Normal_velocity_robot_=Q_.transpose()*DX_;
		NF_=(N_.transpose()*F_)(0,0);

		if (Normal_velocity_robot_(0)<delta_dx_)
		{
			Lambda_(0,0)=(delta_dx_-Normal_velocity_robot_(0)+exp(-Gamma_Value_/epsilon))/(Gamma_Value_*NF_);
			cout<<"It is going faster! "<<Lambda_(0,0)<<" "<<Normal_velocity_robot_(0)<<endl;
		}
		else
		{
			F_dNMN_=F_d_*(N_.transpose()*InvMass_*N_)(0,0)/NF_;
			if ((delta_dx_<Normal_velocity_robot_(0))&&(Normal_velocity_robot_(0)<0))
			{
				Lambda_(0,0)=-F_dNMN_*exp(-Gamma_Value_/epsilon);
				cout<<"It is going a slow as it needs to go "<<Lambda_(0,0)<<endl;
			}
			else if (0<=Normal_velocity_robot_(0))
			{
				Lambda_(0,0)=-100*F_dNMN_*(Normal_velocity_robot_(0)+exp(-Gamma_Value_/epsilon));
			//	Lambda_(0,0)=-10;
				cout<<"It is not going to that direction "<<Lambda_(0,0)<<endl;
			}



		}
	}
	else if  (Gamma_Value_<=0)
	{
		NF_=(N_.transpose()*F_)(0,0);
		F_dNMN_=-F_d_*(N_.transpose()*InvMass_*N_)(0,0)/NF_;
		Lambda_(0,0)=F_dNMN_;
		cout<<"The contact is established "<<Lambda_(0,0)<<endl;
	}
	M_=Q_*Lambda_*Q_inv_;


/*	cout<<"Lambda_ "<<endl;cout<<Lambda_<<endl;
	cout<<"M_ "<<endl;cout<<M_<<endl;*/
//	everyfalse();

	return M_;

}

/**
 *   @brief Calculate the normal velocity in three dimensions
 *
 *   @return The robot's normal velocity in three dimensions
 */
VectorXd CoDs::Get_Normal_Velocity()
{
	return Q_.transpose()*DX_;
}

/**
 *   @brief  If something is wrong, it is activated.
 *
 *   @return void
 */
void CoDs::Error()
{


	while(ros::ok())
	{

	}

}

/**
 *   @brief  To check if everything has been correctly initialized or set.
 *
 *   @return A flag, if it is true, life is shiny.
 */
inline bool CoDs::everythingisreceived()
{
/*	bool flag=true;

	flag=State_of_system_is_set_;
	flag=State_of_surface_is_set_;*/

	return ((State_of_system_is_set_)&&(State_of_surface_is_set_));

}

/**
 *   @brief  Set all the Boolens to false.
 *
 *   @return void.
 */
inline void CoDs::everyfalse()
{
	State_of_system_is_set_=false;
	State_of_system_is_set_=false;

}






































/**
 *   @brief  If something is wrong, it is activated.
 *
 *   @return void
 */
void Gamma::Error()
{


	while(ros::ok())
	{

	}

}
