/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "../framework/SeleniumTest.h"

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCheckBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

using namespace Wt;
using namespace Selenium;

namespace {
  class WCheckBoxTestApp : public WApplication
  {
  public:
    WCheckBoxTestApp(const WEnvironment& env)
      : WApplication(env)
    {
      testCheckBox_ = root()->addNew<Wt::WCheckBox>();
      testCheckBox_->setId("test-checkbox");
      testCheckBox_->setChecked(true);

      auto updateText = root()->addNew<Wt::WText>();
      updateText->setId("status-text");

      auto checkBoxStatusText = root()->addNew<Wt::WText>();
      checkBoxStatusText->setId("checkbox-status-text");
      checkBoxStatusText->setText(testCheckBox_->isChecked() ? "checked" : "unchecked");

      testCheckBox_->changed().connect([=] {
        updateText->setText("changed");
        checkBoxStatusText->setText(testCheckBox_->isChecked() ? "checked" : "unchecked");
      });
      testCheckBox_->mouseWentUp().connect([=] {
        updateText->setText("mouse up");
      });
      testCheckBox_->mouseWentDown().connect([=] {
        updateText->setText("mouse down");
      });
    };

    WCheckBox* testCheckBox() { return testCheckBox_; }

  private:
    WCheckBox* testCheckBox_;
  };

  const std::string useFormDataCacheConf = "<server>"
                                             "<application-settings location=\"*\">"
                                               "<cache-form-data>true</cache-form-data>"
                                             "</application-settings>"
                                           "</server>";

  bool isChecked(Element& element)
  {
    return element.isSelected().value_or(false);
  }

  bool isUnchecked(Element& element)
  {
    return !element.isSelected().value_or(true);
  }
}

BOOST_AUTO_TEST_SUITE(selenium_wcheckbox)

SELENIUM_TEST(wcheckbox_initial, WCheckBoxTestApp)
  auto element = api.getElement(SeleniumAPI::FindBy::ID, "test-checkbox");
  BOOST_TEST((element.has_value() && isChecked(*element)));

  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "status-text");
  BOOST_TEST((textElement.has_value() && textElement->text() == ""));

  auto checkBoxStatusElement = api.getElement(SeleniumAPI::FindBy::ID, "checkbox-status-text");
  BOOST_TEST((checkBoxStatusElement.has_value() && checkBoxStatusElement->text() == "checked"));
END_SELENIUM_TEST

SELENIUM_TEST_WITH_CONFIG_FILE(wcheckbox_change_with_form_data_cache, WCheckBoxTestApp, useFormDataCacheConf)
  auto cbElement = api.getElement(SeleniumAPI::FindBy::ID, "test-checkbox");
  BOOST_REQUIRE(cbElement.has_value());
  BOOST_REQUIRE(isChecked(*cbElement));
  auto textElement = api.getElement(SeleniumAPI::FindBy::ID, "status-text");
  BOOST_REQUIRE(textElement.has_value());
  BOOST_REQUIRE(textElement->text() == "");
  auto checkBoxStatusElement = api.getElement(SeleniumAPI::FindBy::ID, "checkbox-status-text");
  BOOST_REQUIRE(checkBoxStatusElement.has_value());
  BOOST_REQUIRE(checkBoxStatusElement->text() == "checked");

  cbElement->click();

  BOOST_REQUIRE(wait.until([&]() -> bool {
    const std::string value = textElement->text();
    return value == "changed" || value == "mouse up" || value == "mouse down";
  }));

  BOOST_TEST(isUnchecked(*cbElement));
  BOOST_TEST(checkBoxStatusElement->text() == "unchecked");

END_SELENIUM_TEST

BOOST_AUTO_TEST_SUITE_END()
