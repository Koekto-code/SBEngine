// 15 mar 2022

#include "object.hpp"


extern glm::dvec3 sphCoords(unsigned num);

extern double g_sunMass; // get variable from main.cpp

static std::ofstream ologfile;

static int fabs_positive;

static float fabs32_c(float n) {
	fabs_positive = (n > 0.f);
	return fabs_positive ? n : -n;
}

static double fabs64_ref(double& n) {
	return n >= 0 ? n : -n;
}

void Object::set_mass(float m, glm::vec3* ptr, uint16_t q)
{
	assert(!ptr == !q);
	mass = m;
	
	if (q) {
		mpts = q;
		m_unit = m / mpts;
		init_masspts = ptr;
		masspts = new glm::vec3[q];
		for (uint16_t m = 0; m < q; ++m) {
			masspts[m] = init_masspts[m];
		}
	}
}

void Object::set_surface(float* source, size_t len, uint8_t stride) {
	init_surface = source;
	surface = new float[len];
	surface_len = len;
	this->stride = stride;

	for (uint32_t i = 0; i < len; ++i)
	{
		surface[i] = init_surface[i];
	}

	r = 0.0;
	for (uint32_t i = 0; i < len; i += stride)
	{
		double len = glm::length(glm::dvec3(
				init_surface[i],
				init_surface[i + 1],
				init_surface[i + 2])
			);
		r = len > r ? len : r;
	}
}

void Object::rotate(const double& dT)
{
	static glm::dvec3 rV = rotateVec;
	if (rotateVec != glm::dvec3(0.0)) {
		if (rotateVec != rV) {
			rvn = glm::normalize(rotateVec);
			rV = rotateVec;
		}

		rotateRads += dT * (rotateVec.x + rotateVec.y + rotateVec.z)
			/ (rvn.x + rvn.y + rvn.z);
		
		while (fabs32_c(rotateRads) > glm::two_pi<double>()) {
			rotateRads += fabs_positive ? -glm::two_pi<double>() : glm::two_pi<double>();
		}
	}

	for (uint32_t i = 0; i < surface_len; i += stride)
	{
		glm::vec3 vert = ph::sp::rotate(rvn, glm::vec3(
				init_surface[i],
				init_surface[i + 1],
				init_surface[i + 2]),
			rotateRads);

		surface[i] = vert.x;
		surface[i + 1] = vert.y;
		surface[i + 2] = vert.z;
	}
	for (uint16_t i = 0; i < mpts; ++i) {
		masspts[i] = ph::sp::rotate(rvn, init_masspts[i], rotateRads);
	}
}

void Object::applyForce(glm::dvec3 force, glm::dvec3 location, const double& dT) // location is relative to object
{
	if (!ph::elenvec3(location, 0.00015)) {
		speed += force * dT;
		return;
	}
	static unsigned char counter = 0;
	
	glm::dvec3 rvec = glm::normalize(glm::cross(force, -location)); // temporary rotating vector
	float rmf = 0.025f; // [kg*m] "rotating difficulty" of object

	// rmf will be different for different rotating axes
	for (uint16_t i = 0; i < mpts; ++i) {
		float tcos = ph::sp::cosvec3(masspts[i], rvec);
		if (tcos < 0.997 && tcos > -0.997)
		rmf += m_unit * 31.41593f * sqrtf(1.f - tcos * tcos) * glm::length(masspts[i]);
	}

	double tcos = ph::cosvec3(force, location);
	double tsin;
	if (tcos < -0.997 || tcos > 0.997)
		tsin = 0.0;
	else {
		tsin = sqrt(1.0 - tcos * tcos);
		if (std::isnan(tsin)) tsin = 0.0;
	}

	rvec *= glm::length(force) * (tsin / rmf);
	rotateVec += rvec * dT;
	speed += fabs64_ref(tcos) * force * dT;
	
	if (!counter++) {
		ologfile.open("latest.log", std::fstream::app);
		ologfile << "rvec: (" << rvec.x << ", " << rvec.y << ", " << rvec.z
		<< ")\ntsin: " << tsin << "; 100tcos: " << tcos * 100.0 << "; 100abs: " << fabs64_ref(tcos) * 100.0 << '\n';
		ologfile.close();
	}
}

void Object::passv(unsigned f, unsigned s, unsigned t, glm::dvec3 vr) {
	surface[f] = surface[s] = surface[t] = vr.x;
	surface[f + 1] = surface[s + 1] = surface[t + 1] = vr.y;
	surface[f + 2] = surface[s + 2] = surface[t + 2] = vr.z;
}

void Object::updatePhysics(const double& dT, bool keypressed) // deltaTime
{
	passv(40, 0, 100, sphCoords(3));
	passv(45, 15, 90, sphCoords(2));
	passv(50, 35, 85, sphCoords(1));
	passv(55, 30, 105, sphCoords(0));
	
	passv(60, 25, 110, sphCoords(4));
	passv(65, 20, 80, sphCoords(5));
	passv(70, 10, 95, sphCoords(6));
	passv(75, 5, 115, sphCoords(7));
	
	coords
	=	(glm::vec3(surface[40], surface[41], surface[42])
	+	glm::vec3(surface[65], surface[66], surface[67]))
	* 	0.5f;
	
	for (unsigned i = 0; i < 120; i += stride) {
		surface[i] -= coords.x;
		surface[i + 1] -= coords.y;
		surface[i + 2] -= coords.z;
	}
	// speed += ph::gravitacc(coords, glm::dvec3(0.0), g_sunMass) * dT; // dvec3(0.0) == sunCoords
	// coords += speed * dT;

	// if (keypressed)
	//	applyForce(glm::dvec3(0.0, 1000.0, 0.0), glm::dvec3(0.1, 0.0, 0.1), dT);
	
	// rotate(dT);
}