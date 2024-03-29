// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
 * Javascript for DeviceTable UI, served from chrome://bluetooth-internals/.
 */
import {assert} from 'chrome://resources/js/assert.m.js';
import {define as crUiDefine} from 'chrome://resources/js/cr/ui.m.js';
import {$} from 'chrome://resources/js/util.m.js';

import {DeviceInfo} from './device.mojom-webui.js';
import {DeviceCollection} from './device_collection.js';
import {formatManufacturerDataMap, formatServiceUuids} from './device_utils.js';

const COLUMNS = {
  NAME: 0,
  ADDRESS: 1,
  RSSI: 2,
  MANUFACTURER_DATA: 3,
  SERVICE_UUIDS: 4,
  CONNECTION_STATE: 5,
  LINKS: 6,
};

/**
 * A table that lists the devices and responds to changes in the given
 * DeviceCollection. Fires events for inspection requests from listed
 * devices.
 * @constructor
 * @extends {HTMLTableElement}
 */
export const DeviceTable = crUiDefine(function() {
  return document.importNode($('table-template').content.children[0], true);
});

DeviceTable.prototype = {
  __proto__: HTMLTableElement.prototype,

  /** @private {?DeviceCollection} */
  devices_: null,

  /**
   * Decorates an element as a UI element class. Caches references to the
   *    table body and headers.
   */
  decorate() {
    /** @private */
    this.body_ = this.tBodies[0];
    /** @private */
    this.headers_ = this.tHead.rows[0].cells;
    /** @private {!Map<!DeviceInfo, boolean>} */
    this.inspectionMap_ = new Map();
  },

  /**
   * Sets the tables device collection.
   * @param {!DeviceCollection} deviceCollection
   */
  setDevices(deviceCollection) {
    assert(!this.devices_, 'Devices can only be set once.');

    this.devices_ = deviceCollection;
    this.devices_.addEventListener('sorted', this.redraw_.bind(this));
    this.devices_.addEventListener('change', this.handleChange_.bind(this));
    this.devices_.addEventListener('splice', this.handleSplice_.bind(this));

    this.redraw_();
  },

  /**
   * Updates the inspect status of the row matching the given |deviceInfo|.
   * If |isInspecting| is true, the forget link is enabled otherwise it's
   * disabled.
   * @param {!DeviceInfo} deviceInfo
   * @param {boolean} isInspecting
   */
  setInspecting(deviceInfo, isInspecting) {
    this.inspectionMap_.set(deviceInfo, isInspecting);
    this.updateRow_(deviceInfo, this.devices_.indexOf(deviceInfo));
  },

  /**
   * Fires a forget pressed event for the row |index|.
   * @param {number} index
   * @private
   */
  handleForgetClick_(index) {
    const event = new CustomEvent('forgetpressed', {
      bubbles: true,
      detail: {
        address: this.devices_.item(index).address,
      }
    });
    this.dispatchEvent(event);
  },

  /**
   * Updates table row on change event of the device collection.
   * @param {!Event} event
   * @private
   */
  handleChange_(event) {
    this.updateRow_(
        /** @type {!DeviceInfo} */ (this.devices_.item(event.index)),
        event.index);
  },

  /**
   * Fires an inspect pressed event for the row |index|.
   * @param {number} index
   * @private
   */
  handleInspectClick_(index) {
    const event = new CustomEvent('inspectpressed', {
      bubbles: true,
      detail: {
        address: this.devices_.item(index).address,
      }
    });
    this.dispatchEvent(event);
  },

  /**
   * Updates table row on splice event of the device collection.
   * @param {!Event} event
   * @private
   */
  handleSplice_(event) {
    event.removed.forEach(function() {
      this.body_.deleteRow(event.index);
    }, this);

    event.added.forEach(function(device, index) {
      this.insertRow_(device, event.index + index);
    }, this);
  },

  /**
   * Inserts a new row at |index| and updates it with info from |device|.
   * @param {!DeviceInfo} device
   * @param {?number} index
   * @private
   */
  insertRow_(device, index) {
    const row = this.body_.insertRow(index);
    row.id = device.address;

    for (let i = 0; i < this.headers_.length; i++) {
      // Skip the LINKS column. It has no data-field attribute.
      if (i === COLUMNS.LINKS) {
        continue;
      }
      row.insertCell();
    }

    // Make two extra cells for the inspect link and connect errors.
    const inspectCell = row.insertCell();

    const inspectLink = document.createElement('a', {is: 'action-link'});
    inspectLink.setAttribute('is', 'action-link');
    inspectLink.textContent = 'Inspect';
    inspectCell.appendChild(inspectLink);
    inspectLink.addEventListener('click', function() {
      this.handleInspectClick_(row.sectionRowIndex);
    }.bind(this));

    const forgetLink = document.createElement('a', {is: 'action-link'});
    forgetLink.setAttribute('is', 'action-link');
    forgetLink.textContent = 'Forget';
    inspectCell.appendChild(forgetLink);
    forgetLink.addEventListener('click', function() {
      this.handleForgetClick_(row.sectionRowIndex);
    }.bind(this));

    this.updateRow_(device, row.sectionRowIndex);
  },

  /**
   * Deletes and recreates the table using the cached |devices_|.
   * @private
   */
  redraw_() {
    this.removeChild(this.body_);
    this.appendChild(document.createElement('tbody'));
    this.body_ = this.querySelector('tbody');
    this.body_.classList.add('table-body');

    for (let i = 0; i < this.devices_.length; i++) {
      this.insertRow_(
          /** @type {!DeviceInfo} */ (this.devices_.item(i)), null);
    }
  },

  /**
   * Updates the row at |index| with the info from |device|.
   * @param {!DeviceInfo} device
   * @param {number} index
   * @private
   */
  updateRow_(device, index) {
    const row = this.body_.rows[index];
    assert(row, 'Row ' + index + ' is not in the table.');

    row.classList.toggle('removed', this.devices_.isRemoved(device));

    const forgetLink = row.cells[COLUMNS.LINKS].children[1];

    if (this.inspectionMap_.has(device)) {
      forgetLink.disabled = !this.inspectionMap_.get(device);
    } else {
      forgetLink.disabled = true;
    }

    // Update the properties based on the header field path.
    for (let i = 0; i < this.headers_.length; i++) {
      // Skip the LINKS column. It has no data-field attribute.
      if (i === COLUMNS.LINKS) {
        continue;
      }

      const header = this.headers_[i];
      const propName = header.dataset.field;

      const parts = propName.split('.');
      let obj = device;
      while (obj != null && parts.length > 0) {
        const part = parts.shift();
        obj = obj[part];
      }

      if (propName == 'isGattConnected') {
        obj = obj ? 'Connected' : 'Not Connected';
      } else if (propName == 'serviceUuids') {
        obj = formatServiceUuids(obj);
      } else if (propName == 'manufacturerDataMap') {
        obj = formatManufacturerDataMap(obj);
      }

      const cell = row.cells[i];
      cell.textContent = obj == null ? 'Unknown' : obj;
      cell.dataset.label = header.textContent;
    }
  },
};
