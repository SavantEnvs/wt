/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WBootstrap5Theme.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WMenu.h>
#include <Wt/WMenuItem.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WTabWidget.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>

#include <cstdlib>
#include <memory>
#include <optional>
#include <string>

using namespace Wt;
using namespace Selenium;

namespace {
  constexpr int heightTolerance = 4;

  std::unique_ptr<WContainerWidget> createResizableLayout(const std::string& idPrefix)
  {
    auto container = std::make_unique<WContainerWidget>();
    container->setId(idPrefix + "_layout");
    container->resize(WLength(100, LengthUnit::Percentage),
                      WLength(100, LengthUnit::Percentage));

    auto verticalLayout = container->setLayout(std::make_unique<WVBoxLayout>());
    verticalLayout->setContentsMargins(0, 0, 0, 0);

    auto horizontalLayout = verticalLayout->addLayout(std::make_unique<WHBoxLayout>(), 1);
    horizontalLayout->setPreferredImplementation(LayoutImplementation::JavaScript);
    horizontalLayout->setContentsMargins(0, 0, 0, 0);

    auto bluePane = horizontalLayout->addWidget(std::make_unique<WContainerWidget>());
    bluePane->setId(idPrefix + "_blue_pane");
    bluePane->setAttributeValue("style", "background-color: blue;");
    bluePane->setMinimumSize(WLength::Auto, WLength(50, LengthUnit::Pixel));
    bluePane->addNew<WText>("Blue pane");

    auto greenPane = horizontalLayout->addWidget(std::make_unique<WContainerWidget>());
    greenPane->setId(idPrefix + "_green_pane");
    greenPane->setAttributeValue("style", "background-color: green;");
    greenPane->setMinimumSize(WLength::Auto, WLength(50, LengthUnit::Pixel));
    greenPane->addNew<WText>("Green pane");

    horizontalLayout->setResizable(0, true,
                                   WLength(70, LengthUnit::Percentage));

    return container;
  }

  void configureApplication(WApplication& application)
  {
    application.setTheme(std::make_shared<WBootstrap5Theme>());
    application.root()->resize(800, 600);
  }

  class DeferredWidget : public WContainerWidget
  {
  public:
    DeferredWidget()
    {
      setId("deferred_widget");
      setHeight(WLength(100, LengthUnit::Percentage));
    }

  private:
    void load() override
    {
      addWidget(createResizableLayout("deferred"));
      WContainerWidget::load();
    }
  };

  class DeferredStackedWidgetApp : public WApplication
  {
  public:
    DeferredStackedWidgetApp(const WEnvironment& environment)
      : WApplication(environment)
    {
      configureApplication(*this);

      auto applicationLayout = root()->setLayout(std::make_unique<WHBoxLayout>());

      auto navigation = applicationLayout->addWidget(
        std::make_unique<WContainerWidget>());
      navigation->setWidth(WLength(280, LengthUnit::Pixel));

      auto content = applicationLayout->addWidget(
        std::make_unique<WContainerWidget>(), 1);
      auto contentLayout = content->setLayout(std::make_unique<WVBoxLayout>());

      auto stack = contentLayout->addWidget(std::make_unique<WStackedWidget>(), 1);
      stack->setId("deferred_content_region");

      auto menu = navigation->addNew<WMenu>(stack);
      menu->addItem("Text", std::make_unique<WText>("Ordinary text"));
      auto deferredItem = menu->addItem("Deferred layout",
                                        std::make_unique<DeferredWidget>());
      deferredItem->anchor()->setId("deferred-menu-anchor");
    }
  };

  class LazyTabWidgetApp : public WApplication
  {
  public:
    LazyTabWidgetApp(const WEnvironment& environment)
      : WApplication(environment)
    {
      configureApplication(*this);

      auto applicationLayout = root()->setLayout(std::make_unique<WHBoxLayout>());

      auto navigation = applicationLayout->addWidget(
        std::make_unique<WContainerWidget>());
      navigation->setWidth(WLength(280, LengthUnit::Pixel));

      auto tabs = applicationLayout->addWidget(std::make_unique<WTabWidget>(), 1);
      tabs->setId("lazy_tabs");
      tabs->contentsStack()->setId("lazy_content_region");
      tabs->addTab(std::make_unique<WText>("First tab"), "First",
                   ContentLoading::Lazy);
      tabs->addTab(std::make_unique<WText>("Second tab"), "Second",
                   ContentLoading::Lazy);
      auto layoutTab = tabs->addTab(createResizableLayout("lazy"), "Layout",
                                    ContentLoading::Lazy);
      layoutTab->anchor()->setId("lazy-third-tab-anchor");
    }
  };

  Element waitForVisibleElement(SeleniumAPI& api, SeleniumWait& wait,
                                const std::string& id)
  {
    std::optional<Element> element;
    BOOST_REQUIRE(wait.until([&]() {
      element = api.getElement(SeleniumAPI::FindBy::ID, id);
      return element.has_value() && element->isVisible();
    }));
    return *element;
  }

  void waitForLayoutAndCheckHeight(SeleniumAPI& api, SeleniumWait& wait,
                                   const std::string& regionId,
                                   const std::string& idPrefix)
  {
    waitForVisibleElement(api, wait, idPrefix + "_layout");

    auto region = waitForVisibleElement(api, wait, regionId);
    auto bluePane = waitForVisibleElement(api, wait,
                                          idPrefix + "_blue_pane");
    auto greenPane = waitForVisibleElement(api, wait,
                                           idPrefix + "_green_pane");

    wait.until([&]() {
      const int regionHeight = region.height();
      return regionHeight > 200
        && std::abs(regionHeight - bluePane.height()) <= heightTolerance
        && std::abs(regionHeight - greenPane.height()) <= heightTolerance;
    });

    const int regionHeight = region.height();
    BOOST_TEST(regionHeight > 200);
    BOOST_TEST(std::abs(regionHeight - bluePane.height()) <= heightTolerance);
    BOOST_TEST(std::abs(regionHeight - greenPane.height()) <= heightTolerance);
  }
}

BOOST_AUTO_TEST_SUITE(selenium_wstackedwidget)

SELENIUM_TEST(deferred_content_resizes_layout, DeferredStackedWidgetApp)
  auto menuAnchor = waitForVisibleElement(api, wait, "deferred-menu-anchor");
  BOOST_REQUIRE(menuAnchor.click());
  BOOST_REQUIRE(wait.until([&]() {
    return menuAnchor.className().find("active") != std::string::npos;
  }));

  waitForLayoutAndCheckHeight(api, wait, "deferred_content_region", "deferred");
END_SELENIUM_TEST

SELENIUM_TEST(lazy_tab_resizes_layout, LazyTabWidgetApp)
  auto tabAnchor = waitForVisibleElement(api, wait, "lazy-third-tab-anchor");
  BOOST_REQUIRE(tabAnchor.click());
  BOOST_REQUIRE(wait.until([&]() {
    return tabAnchor.className().find("active") != std::string::npos;
  }));

  waitForLayoutAndCheckHeight(api, wait, "lazy_content_region", "lazy");
END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
