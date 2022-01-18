// 18 jan 2022

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

//static const float cst_G(6.674301e-11f);
//static const double dcst_G(6.67430151e-11);

//static const float cst_c(299792448.f);
//static const double dcst_c(299792448.0);


static glm::vec3 fdist;
static glm::dvec3 dist;

static float ftemp_r_fn_exclength;
static double temp_r_fn_exclength;


float ph_radf() {
	return ftemp_r_fn_exclength;
}

double ph_rad() {
	return temp_r_fn_exclength;
}

glm::vec3 ph_fdist() {
	return fdist;
}

glm::dvec3 ph_dist() {
	return dist;
}

float ph_cosvecf(glm::vec3 v, glm::vec3 av) {
	return glm::dot(v, av) / (glm::length(v) * glm::length(av));
}

double ph_cosvec(glm::dvec3 v, glm::dvec3 av) {
	return glm::dot(v, av) / (glm::length(v) * glm::length(av));
}

float ph_sinvecf(glm::vec3 v, glm::vec3 av) {
	return glm::length(cross(v, av)) / (glm::length(v) * glm::length(av));
}

double ph_sinvec(glm::dvec3 v, glm::dvec3 av) {
	return glm::length(cross(v, av)) / (glm::length(v) * glm::length(av));
}

glm::vec3 ph_rotatefn(glm::vec3 turn_vector/*normalized!!!*/, glm::vec3 rotatif, float degree)
{
	glm::vec3 v = turn_vector * glm::length(rotatif) * ph_cosvecf(turn_vector, rotatif);
	glm::vec3 sub = rotatif - v;
	float delta_sin = sin(degree);
	float delta_cos = cos(degree);//sqrt(1.f - delta_sin * delta_sin);
	//if (degree > 1.570796f && degree < 4.712389f)
		//delta_cos = -delta_cos;
	glm::vec3 rotatif_next = rotatif + glm::normalize(glm::cross(v, sub)) * delta_sin * glm::length(sub) - sub * (1.f - delta_cos);
	return rotatif_next;
}

float ph_rvfv(glm::vec3 v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

glm::vec3 ph_rotatef(glm::vec3 turn_vector/*normalized!!!*/, glm::vec3 rotatif, float degree)
{
	float tcos = ph_cosvecf(turn_vector, rotatif);
	glm::vec3 v = turn_vector * glm::length(rotatif) * tcos;
	glm::vec3 sub = rotatif - v;
	if (tcos < 0.f)
		rotatif += glm::normalize(glm::cross(sub, v)) * sin(degree) * ph_rvfv(sub) - sub * (1.f - cos(degree));
	else rotatif += glm::normalize(glm::cross(v, sub)) * sin(degree) * ph_rvfv(sub) - sub * (1.f - cos(degree));
	return rotatif;
}

glm::dvec3 ph_rotate(glm::dvec3 turn_vector/*normalized!!!*/, glm::dvec3 rotatif, double degree)
{
	glm::dvec3 v = turn_vector * glm::length(rotatif) * ph_cosvec(turn_vector, rotatif);
	glm::dvec3 sub = rotatif - v;
	return rotatif + glm::normalize(cross(turn_vector, sub)) * sin(degree) * glm::length(sub) - sub * (1.0 - cos(degree));
}

bool ph_exclengthf(glm::vec3 v, glm::vec3 av, float len) // if distance between bodies exceed (len) distance
{
	fdist = av - v;
	if (abs(fdist.x) > len || abs(fdist.y) > len || abs(fdist.z) > len)
		return true;
	ftemp_r_fn_exclength = glm::length(fdist);
	return ftemp_r_fn_exclength > len;
}

bool ph_exclength(glm::dvec3 v, glm::dvec3 av, double len)
{
	dist = av - v;
	if (abs(dist.x) > len || abs(dist.y) > len || abs(dist.z) > len)
		return true;
	temp_r_fn_exclength = glm::length(dist);
	return temp_r_fn_exclength > len;
}

glm::vec3 ph_gravitaccf(glm::vec3 v, glm::vec3 av, float am)
{
	return ph_exclengthf(v, av, am) ? glm::vec3(0.f, 0.f, 0.f) : fdist * (am / (ftemp_r_fn_exclength * ftemp_r_fn_exclength * ftemp_r_fn_exclength));
}

glm::dvec3 ph_gravitacc(glm::dvec3 v, glm::dvec3 av, double am)
{
	return ph_exclength(v, av, am) ? glm::dvec3(0.0, 0.0, 0.0) : dist * (am / (temp_r_fn_exclength * temp_r_fn_exclength * temp_r_fn_exclength));
}