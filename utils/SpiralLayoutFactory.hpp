#ifndef SPIRALLAYOUTFACTORY_HPP
#define SPIRALLAYOUTFACTORY_HPP

#include "AbstractLayoutFactory.hpp"
#include "layouts/SpiralLayout.hpp"


class SpiralLayoutFactory : public AbstractLayoutFactory {
public:
    SpiralLayoutFactory() = default;

    SpiralLayout* makeLayout() { return new SpiralLayout; }
};

#endif // SPIRALLAYOUTFACTORY_HPP
