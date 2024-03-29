// Copyright 2022 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/autofill/core/browser/payments/test_virtual_card_enrollment_manager.h"
#include "components/autofill/core/browser/payments/virtual_card_enrollment_flow.h"
#include "components/autofill/core/browser/test_personal_data_manager.h"

namespace autofill {

TestVirtualCardEnrollmentManager::TestVirtualCardEnrollmentManager(
    raw_ptr<TestPersonalDataManager> personal_data_manager,
    raw_ptr<payments::TestPaymentsClient> payments_client,
    raw_ptr<TestAutofillClient> autofill_client = nullptr)
    : VirtualCardEnrollmentManager(personal_data_manager,
                                   payments_client,
                                   autofill_client) {}

TestVirtualCardEnrollmentManager::~TestVirtualCardEnrollmentManager() = default;

void TestVirtualCardEnrollmentManager::LoadRiskDataAndContinueFlow(
    raw_ptr<PrefService> user_prefs,
    base::OnceCallback<void(const std::string&)> callback) {
  std::move(callback).Run("some risk data");
}

void TestVirtualCardEnrollmentManager::
    OnDidGetUpdateVirtualCardEnrollmentResponse(
        VirtualCardEnrollmentRequestType type,
        AutofillClient::PaymentsRpcResult result) {
  result_ = result;
  VirtualCardEnrollmentManager::OnDidGetUpdateVirtualCardEnrollmentResponse(
      type, result);
}

void TestVirtualCardEnrollmentManager::Reset() {
  reset_called_ = true;
}

void TestVirtualCardEnrollmentManager::ShowVirtualCardEnrollBubble() {
  bubble_shown_ = true;
}

}  // namespace autofill
