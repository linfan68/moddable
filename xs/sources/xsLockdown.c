/*
 * Copyright (c) 2020-2022  Moddable Tech, Inc.
 *
 *   This file is part of the Moddable SDK Runtime.
 * 
 *   The Moddable SDK Runtime is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 * 
 *   The Moddable SDK Runtime is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with the Moddable SDK Runtime.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This file incorporates work covered by the following copyright and  
 * permission notice:  
 *
 *       Copyright (C) 2010-2016 Marvell International Ltd.
 *       Copyright (C) 2002-2010 Kinoma, Inc.
 *
 *       Licensed under the Apache License, Version 2.0 (the "License");
 *       you may not use this file except in compliance with the License.
 *       You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *       Unless required by applicable law or agreed to in writing, software
 *       distributed under the License is distributed on an "AS IS" BASIS,
 *       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *       See the License for the specific language governing permissions and
 *       limitations under the License.
 */

#include "xsAll.h"
#include "xsScript.h"

void fxSetHostFunctionProperty(txMachine* the, txSlot* property, txCallback call, txInteger length, txID id)
{
	txSlot* home = the->stack;
	txSlot* function = fxNewHostFunction(the, call, length, id, XS_NO_ID);
	txSlot* slot = mxFunctionInstanceHome(function);
	slot->value.home.object = home->value.reference;
	property->kind = the->stack->kind;
	property->value = the->stack->value;
	mxPop();
}

void fx_lockdown(txMachine* the)
{
#define mxHardenBuiltInCall \
	mxPush(mxGlobal); \
	mxPushSlot(harden); \
	mxCall()
#define mxHardenBuiltInRun \
	mxRunCount(1); \
	mxPop()

	txSlot* constructor;
	txSlot* instance;
	txSlot* property;
	txSlot* item;
	txSlot* harden;
	txInteger id;
	
	if (mxProgram.value.reference->flag & XS_DONT_MARSHALL_FLAG)
		mxTypeError("lockdown already called");
	mxProgram.value.reference->flag |= XS_DONT_MARSHALL_FLAG;

	fxDuplicateInstance(the, mxThrowTypeErrorFunction.value.reference);
	constructor = the->stack;
	constructor->value.reference->flag |= XS_CAN_CONSTRUCT_FLAG;
	mxFunctionInstanceCode(constructor->value.reference)->ID = XS_NO_ID; 
	mxFunctionInstanceHome(constructor->value.reference)->value.home.object = NULL;
	
	property = mxBehaviorSetProperty(the, mxAsyncFunctionPrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}
	property = mxBehaviorSetProperty(the, mxAsyncGeneratorFunctionPrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}
	property = mxBehaviorSetProperty(the, mxFunctionPrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}
	property = mxBehaviorSetProperty(the, mxGeneratorFunctionPrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}
	property = mxBehaviorSetProperty(the, mxCompartmentPrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}

	instance = fxNewArray(the, _Compartment);
	property = the->stackIntrinsics - 1;
	item = instance->next->value.array.address;
	for (id = 0; id < XS_SYMBOL_ID_COUNT; id++) {
		*((txIndex*)item) = id;
		property--;
		item++;
	}
	for (; id < _Compartment; id++) {
		*((txIndex*)item) = id;
		item->kind = property->kind;
		item->value = property->value;
		property--;
		item++;
	}
	
	fxDuplicateInstance(the, mxDateConstructor.value.reference);
	property = mxFunctionInstanceCode(the->stack->value.reference);
	property->value.callback.address = mxCallback(fx_Date_secure);
	property = mxBehaviorSetProperty(the, the->stack->value.reference, mxID(_now), 0, XS_OWN);
	fxSetHostFunctionProperty(the, property, mxCallback(fx_Date_now_secure), 0, mxID(_now));
	property = mxBehaviorSetProperty(the, mxDatePrototype.value.reference, mxID(_constructor), 0, XS_OWN);
	if (property) {
		property->kind = constructor->kind;
		property->value = constructor->value;
	}
	mxPull(instance->next->value.array.address[_Date]);
	
	fxDuplicateInstance(the, mxMathObject.value.reference);
	property = mxBehaviorSetProperty(the, the->stack->value.reference, mxID(_random), 0, XS_OWN);
	fxSetHostFunctionProperty(the, property, mxCallback(fx_Math_random_secure), 0, mxID(_random));
#if mxECMAScript2023
	property = mxBehaviorSetProperty(the, the->stack->value.reference, mxID(_irandom), 0, XS_OWN);
	fxSetHostFunctionProperty(the, property, mxCallback(fx_Math_irandom_secure), 0, mxID(_irandom));
#endif	
	mxPull(instance->next->value.array.address[_Math]);

	mxPull(mxCompartmentGlobal);

	mxTemporary(harden);
	mxPush(mxGlobal);
	fxGetID(the, fxID(the, "harden"));
	mxPullSlot(harden);
	
	for (id = XS_SYMBOL_ID_COUNT; id < _Infinity; id++) {
		mxHardenBuiltInCall; mxPush(the->stackIntrinsics[-1 - id]); mxHardenBuiltInRun;
	}
	for (id = _Compartment; id < XS_INTRINSICS_COUNT; id++) {
		mxHardenBuiltInCall; mxPush(the->stackIntrinsics[-1 - id]); mxHardenBuiltInRun;
	}
	
	mxHardenBuiltInCall; mxPush(mxArgumentsSloppyPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxArgumentsStrictPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxArrayIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxAsyncFromSyncIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxAsyncFunctionPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxAsyncGeneratorFunctionPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxAsyncGeneratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxAsyncIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxGeneratorFunctionPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxGeneratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxHostPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxMapIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxModulePrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxRegExpStringIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxSetIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxStringIteratorPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxTransferPrototype); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxTypedArrayPrototype); mxHardenBuiltInRun;

	mxHardenBuiltInCall; mxPush(mxAssignObjectFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxCopyObjectFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxEnumeratorFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxInitializeRegExpFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxOnRejectedPromiseFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxOnResolvedPromiseFunction); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPush(mxOnThenableFunction); mxHardenBuiltInRun;
	
	mxHardenBuiltInCall; mxPushReference(mxArrayLengthAccessor.value.accessor.getter); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPushReference(mxArrayLengthAccessor.value.accessor.setter); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPushReference(mxModuleAccessor.value.accessor.getter); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPushReference(mxStringAccessor.value.accessor.getter); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPushReference(mxStringAccessor.value.accessor.setter); mxHardenBuiltInRun;
	if (mxProxyAccessor.value.accessor.getter) {
		mxHardenBuiltInCall; mxPushReference(mxProxyAccessor.value.accessor.getter); mxHardenBuiltInRun;
	}
	if (mxProxyAccessor.value.accessor.setter) {
		mxHardenBuiltInCall; mxPushReference(mxProxyAccessor.value.accessor.setter); mxHardenBuiltInRun;
	}
	mxHardenBuiltInCall; mxPushReference(mxTypedArrayAccessor.value.accessor.getter); mxHardenBuiltInRun;
	mxHardenBuiltInCall; mxPushReference(mxTypedArrayAccessor.value.accessor.setter); mxHardenBuiltInRun;
	
	mxHardenBuiltInCall; mxPush(mxArrayPrototype); fxGetID(the, mxID(_Symbol_unscopables)); mxHardenBuiltInRun;
	
	mxHardenBuiltInCall; mxPush(mxCompartmentGlobal); mxHardenBuiltInRun;
	
	mxFunctionInstanceCode(mxThrowTypeErrorFunction.value.reference)->ID = XS_NO_ID; 
	mxFunctionInstanceHome(mxThrowTypeErrorFunction.value.reference)->value.home.object = NULL;

	mxPop();
	mxPop();
}

static void fx_hardenQueue(txMachine* the, txSlot* list, txSlot* instance, txFlag flag)
{
	txSlot* item;
	if (instance->flag & flag)
		return;
	item = fxNewSlot(the);
	item->value.reference = instance;
	item->kind = XS_REFERENCE_KIND;
	list->value.list.last->next = item;
	list->value.list.last = item;
}

static void fx_hardenFreezeAndTraverse(txMachine* the, txSlot* reference, txSlot* freeze, txSlot* list, txFlag flag)
{
	txSlot* instance = reference->value.reference;
	txSlot* property;
	txBoolean useIndexes = 1;
	txSlot* at;
// 	txSlot* slot;

	if (!mxBehaviorPreventExtensions(the, instance))
		mxTypeError("extensible object");
		
	property = instance->next;	
	if (property && (property->flag & XS_INTERNAL_FLAG) && (property->kind == XS_TYPED_ARRAY_KIND)) {
		 useIndexes = 0;
	}

	at = fxNewInstance(the);
	mxBehaviorOwnKeys(the, instance, XS_EACH_NAME_FLAG | XS_EACH_SYMBOL_FLAG, at);
	mxPushUndefined();
	property = the->stack;
	while ((at = at->next)) {
		if ((at->value.at.id != XS_NO_ID) || useIndexes) {
			if (mxBehaviorGetOwnProperty(the, instance, at->value.at.id, at->value.at.index, property)) {
				txFlag mask = XS_DONT_DELETE_FLAG;
				property->flag |= XS_DONT_DELETE_FLAG;
				if (property->kind != XS_ACCESSOR_KIND) {
					mask |= XS_DONT_SET_FLAG;
					property->flag |= XS_DONT_SET_FLAG;
				}
				property->kind = XS_UNINITIALIZED_KIND;
				if (!mxBehaviorDefineOwnProperty(the, instance, at->value.at.id, at->value.at.index, property, mask))
					mxTypeError("cannot configure property");
			}
		}
	}
	mxPop();
	mxPop();
	
// 	if (flag == XS_DONT_MODIFY_FLAG) {
// 		property = instance->next;
// 		while (property) {
// 			if (property->flag & XS_INTERNAL_FLAG) {
// 				switch (property->kind) {
// 				case XS_ARRAY_BUFFER_KIND:
// 				case XS_DATE_KIND:
// 				case XS_MAP_KIND:
// 				case XS_SET_KIND:
// 				case XS_WEAK_MAP_KIND:
// 				case XS_WEAK_SET_KIND:
// 					property->flag |= XS_DONT_SET_FLAG;
// 					break;				
// 				case XS_PRIVATE_KIND:
// 					slot = property->value.private.first;
// 					while (slot) {
// 						if (slot->kind != XS_ACCESSOR_KIND) 
// 							slot->flag |= XS_DONT_SET_FLAG;
// 						slot->flag |= XS_DONT_DELETE_FLAG;
// 						slot = slot->next;
// 					}
// 					break;
// 				}
// 			}	
// 			property = property->next;
// 		}	
// 	}
	instance->flag |= flag;

	at = fxNewInstance(the);
	mxBehaviorOwnKeys(the, instance, XS_EACH_NAME_FLAG | XS_EACH_SYMBOL_FLAG, at);
	
	mxTemporary(property);
	mxBehaviorGetPrototype(the, instance, property);
	if (property->kind == XS_REFERENCE_KIND)
		fx_hardenQueue(the, list, property->value.reference, flag);
	
	while ((at = at->next)) {
		if (mxBehaviorGetOwnProperty(the, instance, at->value.at.id, at->value.at.index, property)) {
			if (property->kind == XS_REFERENCE_KIND)
				fx_hardenQueue(the, list, property->value.reference, flag);
			else if (property->kind == XS_ACCESSOR_KIND) {
				if (property->value.accessor.getter)
					fx_hardenQueue(the, list, property->value.accessor.getter, flag);
				if (property->value.accessor.setter)
					fx_hardenQueue(the, list, property->value.accessor.setter, flag);
			}
		}
	}
	
// 	if (flag == XS_DONT_MODIFY_FLAG) {
// 		property = instance->next;
// 		while (property) {
// 			if (property->flag & XS_INTERNAL_FLAG) {
// 				if (property->kind == XS_PRIVATE_KIND) {
// 					txSlot* item = property->value.private.first;
// 					while (item) {
// 						if (property->kind == XS_REFERENCE_KIND)
// 							fx_hardenQueue(the, list, property->value.reference, flag);
// 						else if (property->kind == XS_ACCESSOR_KIND) {
// 							if (property->value.accessor.getter)
// 								fx_hardenQueue(the, list, property->value.accessor.getter, flag);
// 							if (property->value.accessor.setter)
// 								fx_hardenQueue(the, list, property->value.accessor.setter, flag);
// 						}
// 						item = item->next;
// 					}
// 				}
// 				else if (property->kind == XS_DATA_VIEW_KIND) {
// 					property = property->next;
// 					fx_hardenQueue(the, list, property->value.reference, flag);
// 				}
// 			}
// 			property = property->next;
// 		}
// 	}
	
	mxPop();
	mxPop();
}

void fx_harden(txMachine* the)
{
	txFlag flag = XS_DONT_MARSHALL_FLAG;
	txSlot* freeze;
	txSlot* slot;
	txSlot* list;
	txSlot* item;

// 	if (!(mxProgram.value.reference->flag & XS_DONT_MARSHALL_FLAG))
// 		mxTypeError("call lockdown before harden");

	if (mxArgc == 0)
		return;
		
	*mxResult = *mxArgv(0);
		
	slot = mxArgv(0);	
	if (slot->kind != XS_REFERENCE_KIND)
		return;
// 	if (mxArgc > 1) {
// 		txString string = fxToString(the, mxArgv(1));
// 		if (c_strcmp(string, "freeze") == 0)
// 			flag = XS_DONT_MARSHALL_FLAG;
// 		else if (c_strcmp(string, "petrify") == 0)
// 			flag = XS_DONT_MODIFY_FLAG;
// 		else
// 			mxTypeError("invalid integrity");
// 	}
	slot = slot->value.reference;
	if (slot->flag & flag)
		return;

	mxTemporary(freeze);
	mxPush(mxObjectConstructor);
	mxGetID(mxID(_freeze));
	mxPullSlot(freeze);
	
	mxTemporary(list);
	list->value.list.first = C_NULL;	
	list->value.list.last = C_NULL;	
	list->kind = XS_LIST_KIND;
		
	item = fxNewSlot(the);
	item->value.reference = slot;
	item->kind = XS_REFERENCE_KIND;
	list->value.list.first = item;
	list->value.list.last = item;
		
	{
		mxTry(the) {
			while (item) {
				fx_hardenFreezeAndTraverse(the, item, freeze, list, flag);
				item = item->next;
			}
		}
		mxCatch(the) {
			item = list->value.list.first;
			while (item) {
				item->value.reference->flag &= ~flag;
				item = item->next;
			}
			fxJump(the);
		}
	}
		
	mxPop();
	mxPop();
}

void fx_petrify(txMachine* the)
{
	txSlot* slot;
	txSlot* instance;
	txBoolean useIndexes = 1;
	txSlot* at;
	txSlot* property;
	if (mxArgc == 0)
		return;
	slot = mxArgv(0);	
	*mxResult = *slot;
	if (slot->kind != XS_REFERENCE_KIND)
		return;
	instance = slot->value.reference;
	if (!mxBehaviorPreventExtensions(the, instance))
		mxTypeError("extensible object");
	slot = instance->next;	
	if (slot && (slot->flag & XS_INTERNAL_FLAG)) {
		 if ((slot->kind == XS_STRING_KIND) || (slot->kind == XS_STRING_X_KIND) || (slot->kind == XS_TYPED_ARRAY_KIND))
		 	useIndexes = 0;
	}
		
	at = fxNewInstance(the);
	mxBehaviorOwnKeys(the, instance, XS_EACH_NAME_FLAG | XS_EACH_SYMBOL_FLAG, at);
	mxPushUndefined();
	property = the->stack;
	while ((at = at->next)) {
		if ((at->value.at.id != XS_NO_ID) || useIndexes) {
			if (mxBehaviorGetOwnProperty(the, instance, at->value.at.id, at->value.at.index, property)) {
				txFlag mask = XS_DONT_DELETE_FLAG;
				property->flag |= XS_DONT_DELETE_FLAG;
				if (property->kind != XS_ACCESSOR_KIND) {
					mask |= XS_DONT_SET_FLAG;
					property->flag |= XS_DONT_SET_FLAG;
				}
				property->kind = XS_UNINITIALIZED_KIND;
				if (!mxBehaviorDefineOwnProperty(the, instance, at->value.at.id, at->value.at.index, property, mask))
					mxTypeError("cannot configure property");
			}
		}
	}
	mxPop();
	
	property = instance->next;
	while (property) {
		if (property->flag & XS_INTERNAL_FLAG) {
			switch (property->kind) {
			case XS_ARRAY_BUFFER_KIND:
			case XS_DATE_KIND:
			case XS_MAP_KIND:
			case XS_SET_KIND:
			case XS_WEAK_MAP_KIND:
			case XS_WEAK_SET_KIND:
				property->flag |= XS_DONT_SET_FLAG;
				break;				
			case XS_PRIVATE_KIND:
				slot = property->value.private.first;
				while (slot) {
					if (slot->kind != XS_ACCESSOR_KIND) 
						slot->flag |= XS_DONT_SET_FLAG;
					slot->flag |= XS_DONT_DELETE_FLAG;
					slot = slot->next;
				}
				break;
			}
		}	
		property = property->next;
	}	
}

static void fxVerifyCode(txMachine* the, txSlot* list, txSlot* path, txByte* codeBuffer, txSize codeSize);
static void fxVerifyError(txMachine* the, txSlot* path, txID id, txIndex index, txString name);
static void fxVerifyErrorString(txMachine* the, txSlot* slot, txID id, txIndex index, txString name);
static void fxVerifyInstance(txMachine* the, txSlot* list, txSlot* path, txSlot* instance);
static void fxVerifyProperty(txMachine* the, txSlot *list, txSlot *path, txSlot* property, txID id);
static void fxVerifyPropertyError(txMachine* the, txSlot *list, txSlot *path, txSlot* property, txID id, txIndex index);
static void fxVerifyQueue(txMachine* the, txSlot* list, txSlot* path, txSlot* instance, txID id, txIndex index, txString name);

void fx_mutabilities(txMachine* the)
{
	txSlot* instance;
	txSlot* module;
	txSlot* realm;
	txSlot* slot;
	txSlot* list;
	txSlot* item;
	
	fxVars(the, 2);
	
	mxPush(mxArrayPrototype);
	instance = fxNewArrayInstance(the);
	mxPullSlot(mxResult);

	fxNewHostObject(the, NULL);
	fxSetHostChunk(the, the->stack, NULL, the->keyIndex);
	mxPullSlot(mxVarv(0));
	
	module = mxFunctionInstanceHome(mxFunction->value.reference)->value.home.module;
	if (!module) module = mxProgram.value.reference;
	realm = mxModuleInstanceInternal(module)->value.module.realm;
	mxPushSlot(mxRealmGlobal(realm));
	mxPullSlot(mxVarv(1));

	if (mxArgc == 0)
		return;
	slot = mxArgv(0);	
	if (slot->kind != XS_REFERENCE_KIND)
		return;
	
	mxTemporary(list);
	list->value.list.first = C_NULL;	
	list->value.list.last = C_NULL;	
	list->kind = XS_LIST_KIND;
		
	item = fxNewSlot(the);
	item->value.list.first = C_NULL;
	item->value.list.last = slot->value.reference;
	item->kind = XS_LIST_KIND;
	list->value.list.first = item;
	list->value.list.last = item;
		
	{
		mxTry(the) {
			while (item) {
				fxVerifyInstance(the, list, item->value.list.first, item->value.list.last);
				item = item->next;
			}
			item = list->value.list.first;
			while (item) {
				item->value.list.last->flag &= ~XS_LEVEL_FLAG;
				item = item->next;
			}
		}
		mxCatch(the) {
			item = list->value.list.first;
			while (item) {
				item->value.list.last->flag &= ~XS_LEVEL_FLAG;
				item = item->next;
			}
			fxJump(the);
		}
	}
	
	mxPop(); // list
	
	fxCacheArray(the, instance);
	mxPushSlot(mxResult);
	mxPushSlot(mxResult);
	mxGetID(mxID(_sort));
	mxCall();
	mxRunCount(0);
	mxPullSlot(mxResult);
}

void fxVerifyCode(txMachine* the, txSlot* list, txSlot* path, txByte* codeBuffer, txSize codeSize)
{
	const txS1* bytes = gxCodeSizes;
	txByte* p = codeBuffer;
	txByte* q = p + codeSize;
	txU1 byte;
	txS1 offset;
	txID id;
	txInteger count = 0;
	txByte flag = 0;
	txByte* flags = fxGetHostChunk(the, mxVarv(0));
	while (p < q) {
// 		fprintf(stderr, "%s", gxCodeNames[*((txU1*)p)]);
		byte = (txU1)c_read8(p);
		offset = (txS1)c_read8(bytes + byte);
		if (0 < offset) {
			p += offset;
		}
		else if (0 == offset) {
			p++;
			mxDecodeID(p, id);
			if (byte == XS_CODE_PROGRAM_REFERENCE) {
				flag = 1;
				flags[id] = 1;
			}
		}
		else if (-1 == offset) {
			txU1 index;
			p++;
			index = *((txU1*)p);
			p += 1 + index;
		}
        else if (-2 == offset) {
			txU2 index;
            p++;
            mxDecode2(p, index);
            p += index;
        }
        else if (-4 == offset) {
			txS4 index;
            p++;
            mxDecode4(p, index);
            p += index;
        }
// 		fprintf(stderr, "\n");
		if ((XS_CODE_BEGIN_SLOPPY <= byte) && (byte <= XS_CODE_BEGIN_STRICT_FIELD)) {
			count++;
		}
		else if ((XS_CODE_END <= byte) && (byte <= XS_CODE_END_DERIVED)) {
			count--;
			if (count == 0)
				break;
		}
	}
	if (flag) {
		txSlot* instance = fxGetInstance(the, mxVarv(1));
		txSlot* item;
		txSlot* name;
		
		mxTemporary(item);
		
		item->value.list.first = name = fxNewSlot(the);
		item->value.list.last = C_NULL;
		item->kind = XS_LIST_KIND;

		name->value.string = "GlobalEnvironment";
		name->kind = XS_STRING_X_KIND;
		name->next = path;
		
		flags = fxGetHostChunk(the, mxVarv(0));
		id = 0;
		while (id < the->keyIndex) {
			if (flags[id]) {
				txSlot* property = mxBehaviorGetProperty(the, instance, id, 0, XS_OWN);
				if (property)
					fxVerifyProperty(the, list, name, property, id);
				flags = fxGetHostChunk(the, mxVarv(0));
				flags[id]= 0;
			}
			id++;
		}
		
		mxPop();
	}
}

void fxVerifyError(txMachine* the, txSlot* path, txID id, txIndex index, txString string)
{
	txSlot* array;
	txSlot* slot;
	txSlot* stack;
	
	array = mxResult->value.reference->next;
	slot = fxNewSlot(the);
	slot->next = array->next;
	array->next = slot;
	array->value.array.length++;
	fxString(the, slot, "");
	
	stack = the->stack;
	while (path) {
		mxPushSlot(path);
		path = path->next;
	}
	while (the->stack < stack) {
		if (the->stack->kind == XS_STRING_X_KIND) {
			fxVerifyErrorString(the, slot, XS_NO_ID, 0, the->stack->value.string);
		}
		else {
			fxVerifyErrorString(the, slot, the->stack->value.at.id, the->stack->value.at.index, C_NULL);
		}
		mxPop();
	}
	fxVerifyErrorString(the, slot, id, index, string);
	
	
// 	current = path;
// 	next = C_NULL;
// 	previous = C_NULL;
// 	while (current) {
// 		next = current->next;
// 		current->next = previous;
// 		previous = current;
// 		current = next;
// 	}
// 	
// 	path = previous;
// 	current = path;
// 	while (current) {
// 		if (current->kind == XS_STRING_X_KIND) {
// 			fxVerifyErrorString(the, slot, XS_NO_ID, 0, current->value.string);
// 		}
// 		else {
// 			fxVerifyErrorString(the, slot, current->value.at.id, current->value.at.index, C_NULL);
// 		}
// 		current = current->next;
// 	}
// 	fxVerifyErrorString(the, slot, id, index, string);
// 	
// 	current = path;
// 	next = C_NULL;
// 	previous = C_NULL;
// 	while (current) {
// 		next = current->next;
// 		current->next = previous;
// 		previous = current;
// 		current = next;
// 	}
}

void fxVerifyErrorString(txMachine* the, txSlot* slot, txID id, txIndex index, txString string)
{
	if (string) {
		fxConcatStringC(the, slot, "[[");
		fxConcatStringC(the, slot, string);
		fxConcatStringC(the, slot, "]]");
	}
	else if (id != XS_NO_ID) {
		txBoolean adorn;
		txString string = fxGetKeyString(the, id, &adorn);
		c_snprintf(the->nameBuffer, sizeof(the->nameBuffer), "%s", string);
		if (adorn) {
			fxConcatStringC(the, slot, "[Symbol(");
			fxConcatStringC(the, slot, the->nameBuffer);
			fxConcatStringC(the, slot, ")]");
		}
		else {
			fxConcatStringC(the, slot, ".");
			fxConcatStringC(the, slot, the->nameBuffer);
		}
	}
	else {
		fxNumberToString(the, index, the->nameBuffer, sizeof(the->nameBuffer), 0, 0);
		fxConcatStringC(the, slot, "[");
		fxConcatStringC(the, slot, the->nameBuffer);
		fxConcatStringC(the, slot, "]");
	}
}

void fxVerifyInstance(txMachine* the, txSlot* list, txSlot* path, txSlot* instance)
{
	txSlot* property;
	txSlot* prototype;
	
	instance->flag |= XS_LEVEL_FLAG;
	
	if (instance->next && (instance->next->ID == XS_ENVIRONMENT_BEHAVIOR)) {
		property = instance->next->next;
		while (property) {
			if ((property->kind == XS_CLOSURE_KIND) && (property->ID != XS_NO_ID)) { // skip private fields initializers
				txSlot* closure = property->value.closure;
				if (!(closure->flag & XS_DONT_SET_FLAG)) {
					fxVerifyError(the, path, property->ID, 0, C_NULL);
				}
				if (closure->kind == XS_REFERENCE_KIND) {
					fxVerifyQueue(the, list, path, closure->value.reference, property->ID, 0, C_NULL);
				}
			}
			property = property->next;
		}
		return;
	}
	
	if (!(instance->flag & XS_DONT_PATCH_FLAG)) {
		fxVerifyError(the, path, XS_NO_ID, 0, "Extensible");
	}

	prototype = fxGetPrototype(the, instance);
	if (prototype) {
		fxVerifyQueue(the, list, path, prototype, mxID(___proto__), 0, C_NULL);
	}
	
	property = instance->next;
	while (property) {
		if (property->flag & XS_INTERNAL_FLAG) {
			switch (property->kind) {
			case XS_ARRAY_KIND: 
				{
					txSlot* address = property->value.array.address;
					if (address) {
						txIndex index, offset = 0, size = (((txChunk*)(((txByte*)address) - sizeof(txChunk)))->size) / sizeof(txSlot);
						while (offset < size) {
							address = property->value.array.address + offset;
							index = *((txIndex*)address);
							fxVerifyPropertyError(the, list, path, address, XS_NO_ID, index);
							address = property->value.array.address + offset;
							if (address->kind == XS_REFERENCE_KIND)
								fxVerifyQueue(the, list, path, address->value.reference, XS_NO_ID, index, C_NULL);
							else if (address->kind == XS_ACCESSOR_KIND) {
								if (address->value.accessor.getter)
									fxVerifyQueue(the, list, path, address->value.accessor.getter, XS_NO_ID, index, C_NULL);
								address = property->value.array.address + offset;
								if (address->value.accessor.setter)
									fxVerifyQueue(the, list, path, address->value.accessor.setter, XS_NO_ID, index, C_NULL);
							}
							offset++;
						}
					}
				} 
				break;
			case XS_ARRAY_BUFFER_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG)) {
					if (property->value.arrayBuffer.address != C_NULL)
						fxVerifyError(the, path, XS_NO_ID, 0, "ArrayBufferData");
				}
				break;
			case XS_CODE_KIND:
				if (property->value.code.closures)
					fxVerifyQueue(the, list, path, property->value.code.closures, XS_NO_ID, 0, "Environment");
                fxVerifyCode(the, list, path, property->value.code.address, ((txChunk*)(((txByte*)(property->value.code.address)) - sizeof(txChunk)))->size);
				break;
			case XS_CODE_X_KIND:
				break;
			case XS_DATA_VIEW_KIND:
				property = property->next;
				fxVerifyQueue(the, list, path, property->value.reference, XS_NO_ID, 0, "ViewedArrayBuffer");
				break;
			case XS_DATE_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG))
					fxVerifyError(the, path, XS_NO_ID, 0, "DateValue");
				break;
			case XS_REGEXP_KIND:
				break;
			case XS_MAP_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG))
					fxVerifyError(the, path, XS_NO_ID, 0, "MapData");
				break;
			case XS_MODULE_KIND:
				{
					txSlot* exports = mxModuleInstanceExports(instance);
					if (mxIsReference(exports)) {
						txSlot* property = exports->value.reference->next;
						while (property) {
							if (property->value.export.closure) {
								txSlot* closure = property->value.export.closure;
								closure->flag |= XS_DONT_DELETE_FLAG;
								fxVerifyProperty(the, list, path, closure, property->ID);
								closure->flag &= ~XS_DONT_DELETE_FLAG;
							}
							property = property->next;
						}
					}
				}
				break;
			case XS_PRIVATE_KIND:
				{
					txSlot* item = property->value.private.first;
					while (item) {
						fxVerifyProperty(the, list, path, item, item->ID);
						item = item->next;
					}
				}
				break;
			case XS_PROXY_KIND:
				if (property->value.proxy.handler) {
					fxVerifyQueue(the, list, path, property->value.proxy.target, XS_NO_ID, 0, "ProxyHandler");
				}
				if (property->value.proxy.target) {
					fxVerifyQueue(the, list, path, property->value.proxy.target, XS_NO_ID, 0, "ProxyTarget");
				}
				break;
			case XS_SET_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG))
					fxVerifyError(the, path, XS_NO_ID, 0, "SetData");
				break;
			case XS_WEAK_MAP_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG))
					fxVerifyError(the, path, XS_NO_ID, 0, "WeakMapData");
				break;
			case XS_WEAK_SET_KIND:
				if (!(property->flag & XS_DONT_SET_FLAG))
					fxVerifyError(the, path, XS_NO_ID, 0, "WeakSetData");
				break;
			}
		}
		else {
			fxVerifyProperty(the, list, path, property, property->ID);
		}
		property = property->next;
	}
}

void fxVerifyProperty(txMachine* the, txSlot *list, txSlot *path, txSlot* property, txID id)
{
	fxVerifyPropertyError(the, list, path, property, id, 0);
	if (property->kind == XS_REFERENCE_KIND)
		fxVerifyQueue(the, list, path, property->value.reference, id, 0, C_NULL);
	else if (property->kind == XS_ACCESSOR_KIND) {
		if (property->value.accessor.getter)
			fxVerifyQueue(the, list, path, property->value.accessor.getter, id, 0, C_NULL);
		if (property->value.accessor.setter)
			fxVerifyQueue(the, list, path, property->value.accessor.setter, id, 0, C_NULL);
	}
}

void fxVerifyPropertyError(txMachine* the, txSlot *list, txSlot *path, txSlot* property, txID id, txIndex index)
{
	txBoolean immutable = 1;
	if (property->kind != XS_ACCESSOR_KIND) 
		if (!(property->flag & XS_DONT_SET_FLAG))
			immutable = 0;
	if (!(property->flag & XS_DONT_DELETE_FLAG))
		immutable = 0;
	if (!immutable)
		fxVerifyError(the, path, id, index, C_NULL);
}

void fxVerifyQueue(txMachine* the, txSlot* list, txSlot* path, txSlot* instance, txID id, txIndex index, txString string)
{
	txSlot* item;
	txSlot* name;
	if (instance->kind != XS_INSTANCE_KIND)
		return;
	if (instance->flag & XS_LEVEL_FLAG)
		return;
	item = fxNewSlot(the);
	item->value.list.first = C_NULL;
	item->value.list.last = instance;
	item->kind = XS_LIST_KIND;
	list->value.list.last->next = item;
	list->value.list.last = item;
	
	item->value.list.first = name = fxNewSlot(the);
	if (string) {
		name->value.string = string;
		name->kind = XS_STRING_X_KIND;
	}
	else {
		name->value.at.id = id;
		name->value.at.index = index;
		name->kind = XS_AT_KIND;
	}
	name->next = path;
}

void fx_unicodeCompare(txMachine* the)
{
	txString aString;
	txString bString;

	if (mxArgc < 1)
		aString = "undefined";
	else
		aString = fxToString(the, mxArgv(0));
	if (mxArgc < 2)
		bString = "undefined";
	else
		bString = fxToString(the, mxArgv(1));
#ifdef mxMetering
	{
		txSize aLength = fxUnicodeLength(aString, C_NULL);
		txSize bLength = fxUnicodeLength(bString, C_NULL);
		if (aLength < bLength)
			the->meterIndex += aLength;
		else
			the->meterIndex += bLength;
	}
#endif	
	mxResult->value.integer = mxStringUnicodeCompare(aString, bString);
	mxResult->kind = XS_INTEGER_KIND;
}

