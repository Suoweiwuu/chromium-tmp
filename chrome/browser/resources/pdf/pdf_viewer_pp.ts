// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import './elements/viewer-error-dialog.js';
import './elements/viewer-page-indicator.js';
import './elements/shared-vars.js';
import './elements/viewer-zoom-toolbar.js';
import './pdf_viewer_shared_style.js';

import {assertNotReached} from 'chrome://resources/js/assert_ts.js';
import {loadTimeData} from 'chrome://resources/js/load_time_data.m.js';
import {isRTL} from 'chrome://resources/js/util.m.js';
import {html} from 'chrome://resources/polymer/v3_0/polymer/polymer_bundled.min.js';

import {BrowserApi} from './browser_api.js';
import {FittingType} from './constants.js';
import {MessageData, PluginController, PrintPreviewParams} from './controller.js';
import {ViewerPageIndicatorElement} from './elements/viewer-page-indicator.js';
import {ViewerZoomToolbarElement} from './elements/viewer-zoom-toolbar.js';
import {DeserializeKeyEvent, LoadState, SerializeKeyEvent} from './pdf_scripting_api.js';
import {PDFViewerBaseElement} from './pdf_viewer_base.js';
import {DestinationMessageData, DocumentDimensionsMessageData, hasCtrlModifier, MessageObject, shouldIgnoreKeyEvents} from './pdf_viewer_utils.js';
import {ToolbarManager} from './toolbar_manager.js';


type KeyEventData = MessageData&{keyEvent: Object};

type ExtendedKeyEvent = KeyboardEvent&{
  fromScriptingAPI?: boolean,
  fromPlugin?: boolean,
};

export interface PDFViewerPPElement {
  $: {
    content: HTMLDivElement,
    pageIndicator: ViewerPageIndicatorElement,
    sizer: HTMLDivElement,
    zoomToolbar: ViewerZoomToolbarElement,
  };
}

export class PDFViewerPPElement extends PDFViewerBaseElement {
  static get is() {
    return 'pdf-viewer-pp';
  }

  static get template() {
    return html`{__html_template__}`;
  }

  private isPrintPreviewLoadingFinished_: boolean = false;
  private inPrintPreviewMode_: boolean = false;
  private dark_: boolean = false;
  private pluginController_: PluginController|undefined = undefined;
  private toolbarManager_: ToolbarManager|null = null;

  ready() {
    super.ready();
    window.addEventListener('scroll', () => {
      this.pluginController_!.updateScroll(window.scrollX, window.scrollY);
    });
  }

  isNewUiEnabled() {
    return false;
  }

  getBackgroundColor() {
    return PRINT_PREVIEW_BACKGROUND_COLOR;
  }

  init(browserApi: BrowserApi) {
    super.init(
        browserApi, document.documentElement, this.$.sizer, this.$.content);

    this.pluginController_ = PluginController.getInstance();

    this.$.pageIndicator.setViewport(this.viewport);
    this.toolbarManager_ = new ToolbarManager(window, this.$.zoomToolbar);
  }

  handleKeyEvent(e: ExtendedKeyEvent) {
    if (shouldIgnoreKeyEvents() || e.defaultPrevented) {
      return;
    }

    this.toolbarManager_!.hideToolbarAfterTimeout();
    // Let the viewport handle directional key events.
    if (this.viewport.handleDirectionalKeyEvent(e, false)) {
      return;
    }

    switch (e.key) {
      case 'Tab':
        this.toolbarManager_!.showToolbarForKeyboardNavigation();
        return;
      case 'Escape':
        break;  // Ensure escape falls through to the print-preview handler.
      case 'a':
        if (hasCtrlModifier(e)) {
          this.pluginController_!.selectAll();
          // Since we do selection ourselves.
          e.preventDefault();
        }
        return;
      case '\\':
        if (e.ctrlKey) {
          this.$.zoomToolbar.fitToggleFromHotKey();
        }
        return;
    }

    // Give print preview a chance to handle the key event.
    if (!e.fromScriptingAPI) {
      this.sendScriptingMessage(
          {type: 'sendKeyEvent', keyEvent: SerializeKeyEvent(e)});
    } else {
      // Show toolbar as a fallback.
      if (!(e.shiftKey || e.ctrlKey || e.altKey)) {
        this.$.zoomToolbar.show();
      }
    }
  }

  private setBackgroundColorForPrintPreview_() {
    this.pluginController_!.setBackgroundColor(
        this.dark_ ? PRINT_PREVIEW_DARK_BACKGROUND_COLOR :
                     PRINT_PREVIEW_BACKGROUND_COLOR);
  }

  updateUIForViewportChange() {
    // Offset the toolbar position so that it doesn't move if scrollbars appear.
    const hasScrollbars = this.viewport.documentHasScrollbars();
    const scrollbarWidth = this.viewport.scrollbarWidth;
    const verticalScrollbarWidth = hasScrollbars.vertical ? scrollbarWidth : 0;
    const horizontalScrollbarWidth =
        hasScrollbars.horizontal ? scrollbarWidth : 0;

    // Shift the zoom toolbar to the left by half a scrollbar width. This
    // gives a compromise: if there is no scrollbar visible then the toolbar
    // will be half a scrollbar width further left than the spec but if there
    // is a scrollbar visible it will be half a scrollbar width further right
    // than the spec. In LTR layout, the zoom toolbar is on the left
    // left side, but the scrollbar is still on the right, so this is not
    // necessary.
    const zoomToolbar = this.$.zoomToolbar;
    if (isRTL()) {
      zoomToolbar.style.right =
          -verticalScrollbarWidth + (scrollbarWidth / 2) + 'px';
    }
    // Having a horizontal scrollbar is much rarer so we don't offset the
    // toolbar from the bottom any more than what the spec says. This means
    // that when there is a scrollbar visible, it will be a full scrollbar
    // width closer to the bottom of the screen than usual, but this is ok.
    zoomToolbar.style.bottom = -horizontalScrollbarWidth + 'px';

    // Update the page indicator.
    const visiblePage = this.viewport.getMostVisiblePage();
    const pageIndicator = this.$.pageIndicator;
    const lastIndex = pageIndicator.index;
    pageIndicator.index = visiblePage;
    if (this.documentDimensions!.pageDimensions.length > 1 &&
        hasScrollbars.vertical && lastIndex !== undefined) {
      pageIndicator.style.visibility = 'visible';
    } else {
      pageIndicator.style.visibility = 'hidden';
    }

    this.pluginController_!.viewportChanged();
  }

  handleScriptingMessage(message: MessageObject) {
    if (super.handleScriptingMessage(message)) {
      return true;
    }

    if (this.handlePrintPreviewScriptingMessage_(message)) {
      return true;
    }

    if (this.delayScriptingMessage(message)) {
      return true;
    }

    switch (message.data.type.toString()) {
      case 'getSelectedText':
        this.pluginController_!.getSelectedText().then(
            this.sendScriptingMessage.bind(this));
        break;
      case 'selectAll':
        this.pluginController_!.selectAll();
        break;
      default:
        return false;
    }
    return true;
  }

  /**
   * Handle scripting messages specific to print preview.
   * @param message the message to handle.
   * @return true if the message was handled, false otherwise.
   */
  private handlePrintPreviewScriptingMessage_(message: MessageObject): boolean {
    const messageData = message.data;
    switch (messageData.type.toString()) {
      case 'loadPreviewPage':
        const loadData =
            messageData as MessageData & {url: string, index: number};
        this.pluginController_!.loadPreviewPage(loadData.url, loadData.index);
        return true;
      case 'resetPrintPreviewMode':
        const printPreviewData =
            messageData as (MessageData & PrintPreviewParams);
        this.setLoadState(LoadState.LOADING);
        if (!this.inPrintPreviewMode_) {
          this.inPrintPreviewMode_ = true;
          this.isUserInitiatedEvent = false;
          this.forceFit(FittingType.FIT_TO_PAGE);
          this.updateViewportFit(FittingType.FIT_TO_PAGE);
          this.isUserInitiatedEvent = true;
        }

        // Stash the scroll location so that it can be restored when the new
        // document is loaded.
        this.lastViewportPosition = this.viewport.position;
        this.$.pageIndicator.pageLabels = printPreviewData.pageNumbers;

        this.pluginController_!.resetPrintPreviewMode(printPreviewData);
        return true;
      case 'sendKeyEvent':
        const keyEvent =
            DeserializeKeyEvent((message.data as KeyEventData).keyEvent);
        const extendedKeyEvent = keyEvent as ExtendedKeyEvent;
        extendedKeyEvent.fromScriptingAPI = true;
        this.handleKeyEvent(extendedKeyEvent);
        return true;
      case 'hideToolbar':
        this.toolbarManager_!.resetKeyboardNavigationAndHideToolbar();
        return true;
      case 'darkModeChanged':
        this.dark_ =
            (message.data as (MessageData & {darkMode: boolean})).darkMode;
        this.setBackgroundColorForPrintPreview_();
        return true;
      case 'scrollPosition':
        const position = this.viewport.position;
        const positionData =
            message.data as (MessageData & {x: number, y: number});
        position.y += positionData.y;
        position.x += positionData.x;
        this.viewport.setPosition(position);
        return true;
    }

    return false;
  }

  setLoadState(loadState: LoadState) {
    super.setLoadState(loadState);
    if (loadState === LoadState.FAILED) {
      this.isPrintPreviewLoadingFinished_ = true;
    }
  }

  handlePluginMessage(e: CustomEvent) {
    const data = e.detail;
    switch (data.type.toString()) {
      case 'documentDimensions':
        this.setDocumentDimensions(data as DocumentDimensionsMessageData);
        return;
      case 'loadProgress':
        this.updateProgress((data as {progress: number}).progress);
        return;
      case 'navigateToDestination':
        const destinationData = data as DestinationMessageData;
        this.viewport.handleNavigateToDestination(
            destinationData.page, destinationData.x, destinationData.y,
            destinationData.zoom);
        return;
      case 'printPreviewLoaded':
        this.handlePrintPreviewLoaded_();
        return;
      case 'setIsSelecting':
        this.viewportScroller!.setEnableScrolling(
            (data as (MessageData & {isSelecting: boolean})).isSelecting);
        return;
      case 'setSmoothScrolling':
        this.viewport.setSmoothScrolling((data as (MessageData & {
                                            smoothScrolling: boolean
                                          })).smoothScrolling);
        return;
      case 'touchSelectionOccurred':
        this.sendScriptingMessage({
          type: 'touchSelectionOccurred',
        });
        return;
      case 'documentFocusChanged':
        // TODO(crbug.com/1069370): Draw a focus rect around plugin.
        return;
      case 'sendKeyEvent':
        const keyEvent = DeserializeKeyEvent((data as KeyEventData).keyEvent) as
            ExtendedKeyEvent;
        keyEvent.fromPlugin = true;
        this.handleKeyEvent(keyEvent);
        return;
      case 'beep':
      case 'formFocusChange':
      case 'getPassword':
      case 'metadata':
      case 'navigate':
      case 'setIsEditing':
        // These messages are not relevant in Print Preview.
        return;
    }
    assertNotReached('Unknown message type received: ' + data.type);
  }

  /**
   * Handles a notification that print preview has loaded from the
   * current controller.
   */
  private handlePrintPreviewLoaded_() {
    this.isPrintPreviewLoadingFinished_ = true;
    this.sendDocumentLoadedMessage();
  }

  readyToSendLoadMessage() {
    return this.isPrintPreviewLoadingFinished_;
  }

  forceFit(view: FittingType) {
    this.$.zoomToolbar.forceFit(view);
  }

  handleStrings(strings: {[key: string]: string}) {
    super.handleStrings(strings);
    if (!strings) {
      return;
    }
    this.setBackgroundColorForPrintPreview_();
  }

  updateProgress(progress: number) {
    super.updateProgress(progress);
    if (progress === 100) {
      this.toolbarManager_!.hideToolbarAfterTimeout();
    }
  }
}

/**
 * The background color used for print preview (--google-grey-300). Keep
 * in sync with `ChromePdfStreamDelegate::MapToOriginalUrl()`.
 */
const PRINT_PREVIEW_BACKGROUND_COLOR: number = 0xffdadce0;

/**
 * The background color used for print preview when dark mode is enabled
 * (--google-grey-700).
 */
const PRINT_PREVIEW_DARK_BACKGROUND_COLOR: number = 0xff5f6368;

declare global {
  interface HTMLElementTagNameMap {
    'pdf-viewer-pp': PDFViewerPPElement;
  }
}

customElements.define(PDFViewerPPElement.is, PDFViewerPPElement);
