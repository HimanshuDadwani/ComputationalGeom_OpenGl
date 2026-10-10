#ifndef INVERTED_PENDULUM_H_
#define INVERTED_PENDULUM_H_

//#include "Eigen/Dense"
#include <vector>
#include <cmath>

#define M_PI       3.14159265358979323846
#define M_PI_4     0.785398163397448309616  // pi/4

class InvertedPendulum {
public:
    /**
     * Constructor with parameters and initial conditions
     * @param M Mass of the base
     * @param m Mass of the pendulum
     * @param J Moment of inertia of the pendulum
     * @param l Distance from the base to the pendulum
     * @param c Coefficient of viscous friction (base)
     * @param gamma Coefficient of viscous friction (pendulum)
     * @param x_0 Initial conditions
     */
    InvertedPendulum(double M, double m, double J, double l, double c,
        double gamma, std::vector<double> x_0);
    ~InvertedPendulum();
    /**
     * Constructor with default parameters and initial conditions
     * @param x_0 Initial conditions
     */
    InvertedPendulum(std::vector<double> x_0);

    /**
     * Constructor with default parameters and default initial conditions
     */
    InvertedPendulum();

    /**
     * Updates the state by using the inverted pendulum equations
     * @param time Current simulation time
     * @param u Force applied to the base of the system
     */
    void Update(double time, double u);

    /**
     * Returns the value of the state vector
     */
    std::vector<double> GetState() const;

    void drawCircle();
    void drawLine();
    ///**
    // * Linearizes the system around the equilibrium point (theta -> 0)
    // */
    //void Linearize();

    const double M_;      // mass of the base
    const double m_;      // mass of the pendulum
    const double J_;      // moment of inertia of the pendulum
    const double l_;      // distance from the base to the pendulum
    const double c_;      // coefficient of viscous friction (base)
    const double gamma_;  // coefficient of viscous friction (pendulum)
    const double g_;      // acceleration due to gravity
    const double M_t_;    // total mass
    const double J_t_;    // total inertia

    //std::MatrixXd A_;  // dynamics matrix
    //std::MatrixXd B_;  // control matrix
    //std::MatrixXd C_;  // sensor matrix
    //std::MatrixXd D_;  // direct term

private:
    std::vector<double> x_;      // state vector
    std::vector<double> x_dot_;  // state vector derivative
    double previous_time_;


private:
    unsigned int m_CircleVBO;
    unsigned int m_LineVBO;
};

#endif  // INVERTED_PENDULUM_H_
