#include "invertedPendulum.h"
#include <GLCore.h>
#include <GLCoreUtils.h>

// Vertex struct
typedef struct vertex {
    float x, y, z;
} vertex;

//#include "Eigen/Dense"

InvertedPendulum::InvertedPendulum(double M, double m, double J, double l,
    double c, double gamma, std::vector<double> x_0)
    : M_(M),
    m_(m),
    J_(J),
    l_(l),
    c_(c),
    gamma_(gamma),
    g_(9.81),
    M_t_(M + m),
    J_t_(J + m * std::pow(l, 2)),
    x_( x_0 ),
    x_dot_{ 0, 0, 0, 0 },
    previous_time_(0) 
{
    x_dot_ = { 0, 0, 0, 0 };
    //glGenVertexArrays(1, &m_LineVAO);
    glGenBuffers(1, &m_LineVBO);


    //glGenVertexArrays(1, &m_CircleVAO);
    glGenBuffers(1, &m_CircleVBO);
}

InvertedPendulum::InvertedPendulum(std::vector<double> x_0)
    : InvertedPendulum(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, x_0) {}

InvertedPendulum::~InvertedPendulum()
{
    //glDeleteVertexArrays(1, &m_CircleVAO);
    glDeleteBuffers(1, &m_CircleVBO);
    //glDeleteVertexArrays(1, &m_LineVAO);
    glDeleteBuffers(1, &m_LineVBO);
}

InvertedPendulum::InvertedPendulum()
    : InvertedPendulum(std::vector<double>(4)) {
    x_ = { 0, 0.5, 0, 0 };
}

void InvertedPendulum::Update(double time, double u) {
    // Recover state parameters
    double x = x_[0]; // position of the base
    double theta = x_[1]; // angle of the pendulum
    double vx = x_[2]; // velocity of the base
    double omega = x_[3]; // angular rate of the pendulum

    // Compute common terms
    double s_t = std::sin(theta);
    double c_t = std::cos(theta);
    double o_2 = std::pow(omega, 2);
    double l_2 = std::pow(l_, 2);

    // Calculate derivatives
    x_dot_[0] = vx;
    x_dot_[1] = omega;
    x_dot_[2] = (-m_ * l_ * s_t * o_2 + m_ * g_ * (m_ * l_2 / J_t_) * s_t * c_t -
        c_ * vx - (gamma_ / J_t_) * m_ * l_ * c_t * omega + u) /
        (M_t_ - m_ * (m_ * l_2 / J_t_) * c_t * c_t);
    x_dot_[3] =
        (-m_ * l_2 * s_t * c_t * o_2 + M_t_ * g_ * l_ * s_t - c_ * l_ * c_t * vx -
            gamma_ * (M_t_ / m_) * omega + l_ * c_t * u) /
        (J_t_ * (M_t_ / m_) - m_ * (l_ * c_t) * (l_ * c_t));

    // Apply Euler method to solve differential equations
    double dt = time;// -previous_time_;
    previous_time_ += time;
    std::cout << "total time: " << previous_time_ << std::endl;

    x_[0] += x_dot_[0] * dt;
    x_[1] += x_dot_[1] * dt;
    x_[2] += x_dot_[2] * dt;
    x_[3] += x_dot_[3] * dt;


}

std::vector<double> InvertedPendulum::GetState() const { return x_; }

//void InvertedPendulum::Linearize() {
//    const double mu = M_t_ * J_t_ - std::pow((m_ * l_), 2);
//
//    A_ = Eigen::MatrixXd::Zero(4, 4);
//    A_(0, 2) = 1;
//    A_(1, 3) = 1;
//    A_(2, 1) = std::pow((m_ * l_), 2) * g_ / mu;
//    A_(2, 2) = -c_ * J_t_ / mu;
//    A_(2, 3) = -gamma_ * l_ * m_ / mu;
//    A_(3, 1) = M_t_ * m_ * g_ * l_ / mu;
//    A_(3, 2) = -c_ * l_ * m_ / mu;
//    A_(3, 3) = -gamma_ * M_t_ / mu;
//
//    B_ = Eigen::MatrixXd::Zero(4, 1);
//    B_(2, 0) = J_t_ / mu;
//    B_(3, 0) = l_ * m_ / mu;
//
//    C_ = Eigen::MatrixXd::Identity(4, 4);
//    D_ = Eigen::MatrixXd::Zero(4, 1);
//}

    // Draw a line
void InvertedPendulum::drawLine() {
    double x = x_[0]; // position of the base
    double theta = x_[1]; // angle of the pendulum
    double s_t = std::sin(theta);
    double c_t = std::cos(theta);
    double y = 0.2;
    float vertices[] = {
      x, y, 0,
      x - l_ * s_t, y + l_ * c_t , 0
    };

    //glBindVertexArray(m_LineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_LineVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_LINES, 0, 2);
}

// Draw a circle
void InvertedPendulum::drawCircle() {
    double x = x_[0]; // position of the base
    double theta = x_[1]; // angle of the pendulum
    double s_t = std::sin(theta);
    double c_t = std::cos(theta);
    double y = 0.2;

    vertex c = { x - l_ * s_t, y + l_ * c_t , 0 };
    float radius = 0.05f;
    const int iNumTriangles = 10;
    float vertices[iNumTriangles * 3];
    for (int i = 0; i < iNumTriangles; i++) {
        vertex v = { 0, 0 , 0 };
        v.x = c.x + radius * cos(i * (360 / iNumTriangles) * M_PI / 180);
        v.y = c.y + radius * sin(i * (360 / iNumTriangles) * M_PI / 180);
        vertices[i * 3] = v.x;
        vertices[i * 3 + 1] = v.y;
        vertices[i * 3 + 2] = v.z;
    }

    //glBindVertexArray(m_CircleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_CircleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glDrawArrays(GL_TRIANGLE_FAN, 0, iNumTriangles);
}