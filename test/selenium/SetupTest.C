/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "framework/SeleniumTest.h"

namespace {
  const std::string useNonceConf = "<server>"
                                     "<application-settings location=\"*\">"
                                       "<use-script-nonce>true</use-script-nonce>"
                                     "</application-settings>"
                                   "</server>";

  const std::string noNonceConf = "<server>"
                                    "<application-settings location=\"*\">"
                                      "<use-script-nonce>false</use-script-nonce>"
                                    "</application-settings>"
                                  "</server>";
}

// This test ensures that the SeleniumFixture correctly sets up the
// server, it will call for the API initialization, and load the
// initial page.
SELENIUM_TEST(selenium_setup, Wt::WApplication)
END_SELENIUM_TEST

// Tests that the configuration is correctly applied (in case
// use-script-nonce is false by default).
SELENIUM_TEST_WITH_CONFIG_FILE(selenium_setup_config_file_1, Wt::WApplication, useNonceConf)
  BOOST_REQUIRE(test.config().useScriptNonce());
END_SELENIUM_TEST

// Tests that the configuration is correctly applied (in case
// use-script-nonce is true by default).
SELENIUM_TEST_WITH_CONFIG_FILE(selenium_setup_config_file_2, Wt::WApplication, noNonceConf)
  BOOST_REQUIRE(!test.config().useScriptNonce());
END_SELENIUM_TEST
