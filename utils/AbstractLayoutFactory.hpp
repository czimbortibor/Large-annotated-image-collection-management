#ifndef ABSTRACTLAYOUTFACTORY_HPP
#define ABSTRACTLAYOUTFACTORY_HPP

#include "layouts/AbstractGraphicsLayout.hpp"

class GraphicsView;

class AbstractLayoutFactory {
public:
	/** creates a new AbstractGraphicsLayout instance */
	virtual AbstractGraphicsLayout* makeLayout() = 0;

};

#endif // ABSTRACTLAYOUTFACTORY_HPP
