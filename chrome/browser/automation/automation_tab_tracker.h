// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_AUTOMATION_AUTOMATION_TAB_TRACKER_H_
#define CHROME_BROWSER_AUTOMATION_AUTOMATION_TAB_TRACKER_H_

#include "chrome/browser/automation/automation_resource_tracker.h"
#include "chrome/browser/browser.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/navigation_controller.h"
#include "chrome/common/notification_registrar.h"

class AutomationTabTracker
  : public AutomationResourceTracker<NavigationController*> {
public:
  AutomationTabTracker(IPC::Message::Sender* automation)
    : AutomationResourceTracker(automation) {}

  virtual ~AutomationTabTracker() {
    ClearAllMappings();
  }

  virtual void AddObserver(NavigationController* resource) {
    // This tab could either be a regular tab or an external tab
    // Register for both notifications.
    registrar_.Add(this, NOTIFY_TAB_CLOSING,
                   Source<NavigationController>(resource));
    registrar_.Add(this, NOTIFY_EXTERNAL_TAB_CLOSED,
                   Source<NavigationController>(resource));
    // We also want to know about navigations so we can keep track of the last
    // navigation time.
    registrar_.Add(this, NOTIFY_NAV_ENTRY_COMMITTED,
                   Source<NavigationController>(resource));
  }

  virtual void RemoveObserver(NavigationController* resource) {
    registrar_.Remove(this, NOTIFY_TAB_CLOSING,
                      Source<NavigationController>(resource));
    registrar_.Remove(this, NOTIFY_EXTERNAL_TAB_CLOSED,
                      Source<NavigationController>(resource));
    registrar_.Remove(this, NOTIFY_NAV_ENTRY_COMMITTED,
                      Source<NavigationController>(resource));
  }

  virtual void Observe(NotificationType type,
                       const NotificationSource& source,
                       const NotificationDetails& details) {
    switch (type) {
      case NOTIFY_NAV_ENTRY_COMMITTED:
        last_navigation_times_[Source<NavigationController>(source).ptr()] =
            base::Time::Now();
        return;
      case NOTIFY_EXTERNAL_TAB_CLOSED:
      case NOTIFY_TAB_CLOSING:
        std::map<NavigationController*, base::Time>::iterator iter =
            last_navigation_times_.find(
            Source<NavigationController>(source).ptr());
        if (iter != last_navigation_times_.end())
          last_navigation_times_.erase(iter);
        break;
    }
    AutomationResourceTracker::Observe(type, source, details);
  }

  base::Time GetLastNavigationTime(int handle) {
    if (ContainsHandle(handle)) {
      NavigationController* controller = GetResource(handle);
      if (controller) {
        std::map<NavigationController*, base::Time>::const_iterator iter =
            last_navigation_times_.find(controller);
        if (iter != last_navigation_times_.end())
          return iter->second;
      }
    }
    return base::Time();
  }

 private:
  NotificationRegistrar registrar_;

  // Last time a navigation occurred.
  std::map<NavigationController*, base::Time> last_navigation_times_;

  DISALLOW_COPY_AND_ASSIGN(AutomationTabTracker);
};

#endif  // CHROME_BROWSER_AUTOMATION_AUTOMATION_TAB_TRACKER_H_
