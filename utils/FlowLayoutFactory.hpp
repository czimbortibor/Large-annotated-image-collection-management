#ifndef FLOWLAYOUTFACTORY_HPP
#define FLOWLAYOUTFACTORY_HPP

#include "AbstractLayoutFactory.hpp"
#include "layouts/FlowLayout.hpp"


class FlowLayoutFactory : public AbstractLayoutFactory {
public:
	FlowLayoutFactory() = default;

	FlowLayout* makeLayout() { return new FlowLayout; }
};

#endif // FLOWLAYOUTFACTORY_HPP
