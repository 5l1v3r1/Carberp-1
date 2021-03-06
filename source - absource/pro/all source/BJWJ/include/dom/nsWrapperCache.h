/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Gecko DOM code.
 *
 * The Initial Developer of the Original Code is
 * Mozilla Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2008
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *    Peter Van der Beken <peterv@propagandism.org>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef nsWrapperCache_h___
#define nsWrapperCache_h___

#include "nsCycleCollectionParticipant.h"

typedef unsigned long PtrBits;

#define NS_WRAPPERCACHE_IID \
{ 0x3a51ca81, 0xddab, 0x422c, \
  { 0x95, 0x3a, 0x13, 0x06, 0x28, 0x0e, 0xee, 0x14 } }

/**
 * Class to store the XPCWrappedNative for an object. This can only be used
 * with objects that only have one XPCWrappedNative at a time (usually ensured
 * by setting an explicit parent in the PreCreate hook for the class). This
 * object can be gotten by calling QueryInterface, note that this breaks XPCOM
 * rules a bit (this object doesn't derive from nsISupports).
 */
class nsWrapperCache
{
public:
  NS_DECLARE_STATIC_IID_ACCESSOR(NS_WRAPPERCACHE_IID)

  nsWrapperCache() : mWrapperPtrBits(0)
  {
  }
  ~nsWrapperCache()
  {
    if (PreservingWrapper()) {
      GetWrapper()->Release();
    }
  }

  /**
   * This method returns an nsIXPConnectWrappedNative, but we want to avoid
   * including nsIXPConnect, because we don't want to make everyone require
   * JS and XPConnect.
   */
  nsISupports* GetWrapper()
  {
    return reinterpret_cast<nsISupports*>(mWrapperPtrBits & ~kWrapperBitMask);
  }

  /**
   * This method takes an nsIXPConnectWrappedNative, but we want to avoid
   * including nsIXPConnect, because we don't want to make everyone require
   * JS and XPConnect.
   */
  void SetWrapper(nsISupports* aWrapper)
  {
    NS_ASSERTION(!mWrapperPtrBits, "Already have a wrapper!");
    mWrapperPtrBits = reinterpret_cast<PtrBits>(aWrapper);
  }

  void ClearWrapper()
  {
    if (PreservingWrapper()) {
      GetWrapper()->Release();
    }
    mWrapperPtrBits = 0;
  }

  void PreserveWrapper()
  {
    NS_ASSERTION(mWrapperPtrBits, "No wrapper to preserve?");
    if (!PreservingWrapper()) {
      NS_ADDREF(reinterpret_cast<nsISupports*>(mWrapperPtrBits));
      mWrapperPtrBits |= WRAPPER_BIT_PRESERVED;
    }
  }

  void ReleaseWrapper()
  {
    if (PreservingWrapper()) {
      nsISupports* wrapper = GetWrapper();
      mWrapperPtrBits = reinterpret_cast<PtrBits>(wrapper);
      NS_RELEASE(wrapper);
    }
  }

  void TraverseWrapper(nsCycleCollectionTraversalCallback &cb)
  {
    if (PreservingWrapper()) {
      NS_CYCLE_COLLECTION_NOTE_EDGE_NAME(cb, "mWrapper");
      cb.NoteXPCOMChild(GetWrapper());
    }
  }

private:
  PRBool PreservingWrapper()
  {
    return mWrapperPtrBits & WRAPPER_BIT_PRESERVED;
  }

  enum { WRAPPER_BIT_PRESERVED = 1 << 0 };
  enum { kWrapperBitMask = 0x1 };

  PtrBits mWrapperPtrBits;
};

NS_DEFINE_STATIC_IID_ACCESSOR(nsWrapperCache, NS_WRAPPERCACHE_IID)

#define NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY                                   \
  if ( aIID.Equals(NS_GET_IID(nsWrapperCache)) ) {                            \
    *aInstancePtr = static_cast<nsWrapperCache*>(this);                       \
    return NS_OK;                                                             \
  }

#endif /* nsWrapperCache_h___ */
