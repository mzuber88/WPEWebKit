/*
 * Copyright (C) 2007 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include "JSSQLTransaction.h"
#include "DOMWindow.h"
#include "ExceptionCode.h"
#include "JSSQLStatementCallback.h"
#include "JSSQLStatementErrorCallback.h"
#include "JSDOMWindowCustom.h"
#include "SQLTransaction.h"
#include "SQLValue.h"
#include <runtime/JSObjectInlines.h>

using namespace JSC;

namespace WebCore {

JSValue JSSQLTransaction::executeSql(ExecState& state)
{
    VM& vm = state.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    if (!state.argumentCount()) {
        setDOMException(&state, SYNTAX_ERR);
        return jsUndefined();
    }

    String sqlStatement = state.argument(0).toString(&state)->value(&state);
    RETURN_IF_EXCEPTION(scope, JSValue());

    // Now assemble the list of SQL arguments
    Vector<SQLValue> sqlValues;
    if (!state.argument(1).isUndefinedOrNull()) {
        JSObject* object = state.argument(1).getObject();
        if (!object) {
            setDOMException(&state, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }

        JSValue lengthValue = object->get(&state, state.propertyNames().length);
        RETURN_IF_EXCEPTION(scope, JSValue());
        unsigned length = lengthValue.toUInt32(&state);
        RETURN_IF_EXCEPTION(scope, JSValue());

        for (unsigned i = 0 ; i < length; ++i) {
            JSValue value = object->get(&state, i);
            RETURN_IF_EXCEPTION(scope, JSValue());

            if (value.isUndefinedOrNull())
                sqlValues.append(SQLValue());
            else if (value.isNumber())
                sqlValues.append(value.asNumber());
            else {
                // Convert the argument to a string and append it
                sqlValues.append(value.toString(&state)->value(&state));
                RETURN_IF_EXCEPTION(scope, JSValue());
            }
        }
    }

    RefPtr<SQLStatementCallback> callback;
    if (!state.argument(2).isUndefinedOrNull()) {
        JSObject* object = state.argument(2).getObject();
        if (!object) {
            setDOMException(&state, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }

        callback = JSSQLStatementCallback::create(object, jsCast<JSDOMGlobalObject*>(globalObject()));
    }

    RefPtr<SQLStatementErrorCallback> errorCallback;
    if (!state.argument(3).isUndefinedOrNull()) {
        JSObject* object = state.argument(3).getObject();
        if (!object) {
            setDOMException(&state, TYPE_MISMATCH_ERR);
            return jsUndefined();
        }

        errorCallback = JSSQLStatementErrorCallback::create(object, jsCast<JSDOMGlobalObject*>(globalObject()));
    }

    propagateException(state, scope, wrapped().executeSQL(sqlStatement, sqlValues, WTFMove(callback), WTFMove(errorCallback)));

    return jsUndefined();
}

} // namespace WebCore
