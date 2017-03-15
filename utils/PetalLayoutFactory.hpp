#ifndef PETALLAYOUTFACTORY_HPP
#define PETALLAYOUTFACTORY_HPP

#include "utils/AbstractLayoutFactory.hpp"
#include "layouts/PetalLayout.hpp"


class PetalLayoutFactory : public AbstractLayoutFactory {
public:
	PetalLayoutFactory() = default;

	PetalLayout* makeLayout() { return new PetalLayout; }
};

#endif // PETALLAYOUTFACTORY_HPP
