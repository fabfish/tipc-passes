#include "Variable.h"
#include "llvm/Support/raw_ostream.h"

std::string Variable::pointsToStr()
{
	std::string s = "{";
	auto set = ptset->getSet();
	for (auto ele = set.begin(); ele != set.end(); ele++)
	{
		s += (*ele)->str() + ", ";
	}
	s += "}";
	return s;
}
