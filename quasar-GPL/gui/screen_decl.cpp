// $Id: screen_decl.cpp,v 1.24 2005/03/13 22:12:10 bpepers Exp $
//
// Copyright (C) 1998-2004 Linux Canada Inc.  All rights reserved.
//
// This file is part of Quasar Accounting
//
// This file may be distributed and/or modified under the terms of the
// GNU General Public License version 2 as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL included in the
// packaging of this file.
//
// Licensees holding a valid Quasar Commercial License may use this file
// in accordance with the Quasar Commercial License Agreement provided
// with the Software in the LICENSE.COMMERCIAL file.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// See http://www.linuxcanada.com or email sales@linuxcanada.com for
// information about Quasar Accounting support and maintenance options.
//
// Contact sales@linuxcanada.com if any conditions of this licensing are
// not clear to you.

#include "screen_decl.h"
#include "main_window.h"
#include "quasar_client.h"
#include "quasar_db.h"

#include <qmap.h>
#include <qmessagebox.h>

#include "account_list.h"
#include "account_master.h"
#include "address_book.h"
#include "adjust_reason_list.h"
#include "adjust_reason_master.h"
#include "aged_payables.h"
#include "aged_receivables.h"
#include "auto_order.h"
#include "card_adjustment.h"
#include "card_allocate.h"
#include "card_transfer.h"
#include "cash_reconcile.h"
#include "change_password.h"
#include "charge_list.h"
#include "charge_master.h"
#include "cheque_customer.h"
#include "cheque_master.h"
#include "cheque_print.h"
#include "cheque_vendor.h"
#include "company_list.h"
#include "company_master.h"
#include "count_list.h"
#include "count_master.h"
#include "customer_master.h"
#include "customer_type_list.h"
#include "customer_type_master.h"
#include "dept_list.h"
#include "dept_master.h"
#include "detailed_statement.h"
#include "discount_list.h"
#include "discount_master.h"
#include "employee_master.h"
#include "expense_list.h"
#include "expense_master.h"
#include "find_tx.h"
#include "general_master.h"
#include "group_list.h"
#include "group_master.h"
#include "hh_receive.h"
#include "hh_send_data.h"
#include "hh_send_program.h"
#include "inquiry.h"
#include "invoice_list.h"
#include "invoice_master.h"
#include "item_adjustment.h"
#include "item_history.h"
#include "item_list.h"
#include "item_master.h"
#include "item_transfer.h"
#include "journal.h"
#include "ledger_transfer.h"
#include "label_batch_list.h"
#include "label_batch_master.h"
#include "location_list.h"
#include "location_master.h"
#include "mailing_labels.h"
#include "on_order.h"
#include "open_company.h"
#include "open_balances.h"
#include "order_list.h"
#include "order_master.h"
#include "order_template_list.h"
#include "order_template_master.h"
#include "pat_group_list.h"
#include "pat_group_master.h"
#include "pat_worksheet_list.h"
#include "pat_worksheet_master.h"
#include "pay_bills.h"
#include "payout_master.h"
#include "payout_master.h"
#include "personal_master.h"
#include "price_batch_list.h"
#include "price_batch_master.h"
#include "price_list.h"
#include "price_master.h"
#include "print_stmt.h"
#include "promo_batch_list.h"
#include "promo_batch_master.h"
#include "purchase_report.h"
#include "quasar_config.h"
#include "quote_list.h"
#include "quote_master.h"
#include "receipt_master.h"
#include "receive_list.h"
#include "receive_master.h"
#include "reconcile_list.h"
#include "reconcile_master.h"
#include "recurring_list.h"
#include "recurring_master.h"
#include "report_list.h"
#include "report_tx.h"
#include "sales_history.h"
#include "sales_report.h"
#include "security_type_list.h"
#include "security_type_master.h"
#include "sequence_number.h"
#include "service_charges.h"
#include "slip_list.h"
#include "slip_master.h"
#include "station_list.h"
#include "station_master.h"
#include "stock_status.h"
#include "store_list.h"
#include "store_master.h"
#include "subdept_list.h"
#include "subdept_master.h"
#include "tax_list.h"
#include "tax_master.h"
#include "tender_adjustment.h"
#include "tender_list.h"
#include "tender_master.h"
#include "tender_transfer.h"
#include "term_list.h"
#include "term_master.h"
#include "todo_list.h"
#include "todo_master.h"
#include "user_list.h"
#include "user_master.h"
#include "vendor_master.h"
#include "withdraw_master.h"
#include "year_end_prelim.h"
#include "year_end_final.h"

#include "nosale_view.h"
#include "shift_view.h"

// Create method to instantiate a screen and then register it
#define SCREEN_DECL(name) static void \
create ## name(MainWindow* main) \
{ \
    QWidget* window = new name(main); \
    window->show(); \
} \
static ScreenDecl decl ## name(#name, create ## name);

// Map of screen names to create functions
static QMap<QString,CreateFunc>* screenMap;

// Declarations for every screen
SCREEN_DECL(AccountList);
SCREEN_DECL(AccountMaster);
SCREEN_DECL(AddressBook);
SCREEN_DECL(AdjustReasonList);
SCREEN_DECL(AdjustReasonMaster);
SCREEN_DECL(AgedPayables);
SCREEN_DECL(AgedReceivables);
SCREEN_DECL(AutoOrder);
SCREEN_DECL(CardAdjustment);
SCREEN_DECL(CardAllocate);
SCREEN_DECL(CardTransfer);
SCREEN_DECL(CashReconcile);
SCREEN_DECL(ChangePassword);
SCREEN_DECL(ChargeList);
SCREEN_DECL(ChargeMaster);
SCREEN_DECL(ChequeCustomer);
SCREEN_DECL(ChequeMaster);
SCREEN_DECL(ChequePrint);
SCREEN_DECL(ChequeVendor);
SCREEN_DECL(CompanyList);
SCREEN_DECL(CompanyMaster);
SCREEN_DECL(CountList);
SCREEN_DECL(CountMaster);
SCREEN_DECL(CustomerMaster);
SCREEN_DECL(CustomerTypeList);
SCREEN_DECL(CustomerTypeMaster);
SCREEN_DECL(DeptList);
SCREEN_DECL(DeptMaster);
SCREEN_DECL(DetailedStatement);
SCREEN_DECL(DiscountList);
SCREEN_DECL(DiscountMaster);
SCREEN_DECL(EmployeeMaster);
SCREEN_DECL(ExpenseList);
SCREEN_DECL(ExpenseMaster);
SCREEN_DECL(FindTx);
SCREEN_DECL(GeneralMaster);
SCREEN_DECL(GroupList);
SCREEN_DECL(GroupMaster);
SCREEN_DECL(HandheldReceive);
SCREEN_DECL(HandheldSendData);
SCREEN_DECL(HandheldSendProgram);
SCREEN_DECL(Inquiry);
SCREEN_DECL(InvoiceList);
SCREEN_DECL(InvoiceMaster);
SCREEN_DECL(ItemAdjustment);
SCREEN_DECL(ItemHistory);
SCREEN_DECL(ItemList);
SCREEN_DECL(ItemMaster);
SCREEN_DECL(ItemTransfer);
SCREEN_DECL(Journal)
SCREEN_DECL(LedgerTransfer)
SCREEN_DECL(LabelBatchList)
SCREEN_DECL(LabelBatchMaster)
SCREEN_DECL(LocationList)
SCREEN_DECL(LocationMaster)
SCREEN_DECL(MailingLabels)
SCREEN_DECL(OnOrder)
SCREEN_DECL(OpenCompany)
SCREEN_DECL(OpenBalances)
SCREEN_DECL(OrderList)
SCREEN_DECL(OrderMaster)
SCREEN_DECL(OrderTemplateList)
SCREEN_DECL(OrderTemplateMaster)
SCREEN_DECL(PatGroupList);
SCREEN_DECL(PatGroupMaster);
SCREEN_DECL(PatWorksheetList);
SCREEN_DECL(PatWorksheetMaster);
SCREEN_DECL(PayBills)
SCREEN_DECL(PayoutMaster)
SCREEN_DECL(PersonalMaster)
SCREEN_DECL(PriceBatchList)
SCREEN_DECL(PriceBatchMaster)
SCREEN_DECL(PriceList);
SCREEN_DECL(PriceMaster);
SCREEN_DECL(PrintStmt);
SCREEN_DECL(PromoBatchList)
SCREEN_DECL(PromoBatchMaster)
SCREEN_DECL(PurchaseReport);
SCREEN_DECL(QuasarConfig);
SCREEN_DECL(QuoteList);
SCREEN_DECL(QuoteMaster);
SCREEN_DECL(ReceiptMaster);
SCREEN_DECL(ReceiveList);
SCREEN_DECL(ReceiveMaster);
SCREEN_DECL(ReconcileList);
SCREEN_DECL(ReconcileMaster);
SCREEN_DECL(RecurringList);
SCREEN_DECL(RecurringMaster);
SCREEN_DECL(ReportList);
SCREEN_DECL(ReportTx);
SCREEN_DECL(SalesHistory);
SCREEN_DECL(SalesReport);
SCREEN_DECL(SecurityTypeList)
SCREEN_DECL(SecurityTypeMaster)
SCREEN_DECL(SequenceNumber)
SCREEN_DECL(ServiceCharges)
SCREEN_DECL(SlipList)
SCREEN_DECL(SlipMaster)
SCREEN_DECL(StationList)
SCREEN_DECL(StationMaster)
SCREEN_DECL(StockStatus)
SCREEN_DECL(StoreList)
SCREEN_DECL(StoreMaster)
SCREEN_DECL(SubdeptList)
SCREEN_DECL(SubdeptMaster)
SCREEN_DECL(TaxList)
SCREEN_DECL(TaxMaster)
SCREEN_DECL(TenderAdjustment)
SCREEN_DECL(TenderList)
SCREEN_DECL(TenderMaster)
SCREEN_DECL(TenderTransfer)
SCREEN_DECL(TermList)
SCREEN_DECL(TermMaster)
SCREEN_DECL(TodoList);
SCREEN_DECL(TodoMaster);
SCREEN_DECL(UserList);
SCREEN_DECL(UserMaster);
SCREEN_DECL(VendorMaster);
SCREEN_DECL(WithdrawMaster);
SCREEN_DECL(YearEndPrelim);
SCREEN_DECL(YearEndFinal);

void
ScreenDecl::createScreen(const QString& name, MainWindow* main)
{
    if (screenMap == NULL)
	screenMap = new QMap<QString,CreateFunc>;

    if (!screenMap->contains(name)) {
	qWarning("Unknown screen: " + name);
	return;
    }

    CreateFunc func = (*screenMap)[name];
    func(main);
}

ScreenDecl::ScreenDecl(const QString& name, CreateFunc func)
{
    if (screenMap == NULL)
	screenMap = new QMap<QString,CreateFunc>;

    if (screenMap->contains(name))
	qWarning("Double screen decl: " + name);

    screenMap->insert(name, func);
}

QWidget*
editGltx(const Gltx& gltx, MainWindow* main)
{
    QWidget* edit = NULL;
    Cheque cheque;

    switch (gltx.dataType()) {
    case DataObject::ITEM_ADJUST:
	if (gltx.linkId() == INVALID_ID)
	    edit = new ItemAdjustment(main, gltx.id());
	else
	    edit = new ItemTransfer(main, gltx.id());
	break;
    case DataObject::CHEQUE:
	main->quasar()->db()->lookup(gltx.id(), cheque);
	if (cheque.type() == Cheque::ACCOUNT)
	    edit = new ChequeMaster(main, gltx.id());
	else if (cheque.type() == Cheque::CUSTOMER)
	    edit = new ChequeCustomer(main, gltx.id());
	else
	    edit = new ChequeVendor(main, gltx.id());
	break;
    case DataObject::GENERAL:
	if (gltx.linkId() == INVALID_ID)
	    edit = new GeneralMaster(main, gltx.id());
	else
	    edit = new LedgerTransfer(main, gltx.id());
	break;
    case DataObject::INVOICE:
    case DataObject::RETURN:
	edit = new InvoiceMaster(main, gltx.id());
	break;
    case DataObject::RECEIPT:
	edit = new ReceiptMaster(main, gltx.id());
	break;
    case DataObject::RECEIVE:
    case DataObject::CLAIM:
	edit = new ReceiveMaster(main, gltx.id());
	break;
    case DataObject::WITHDRAW:
	edit = new WithdrawMaster(main, gltx.id());
	break;
    case DataObject::NOSALE:
	edit = new NosaleView(main, gltx.id());
	break;
    case DataObject::PAYOUT:
	edit = new PayoutMaster(main, gltx.id());
	break;
    case DataObject::TEND_ADJUST:
	if (gltx.linkId() == INVALID_ID)
	    edit = new TenderAdjustment(main, gltx.id());
	else
	    edit = new TenderTransfer(main, gltx.id());
	break;
    case DataObject::CARD_ADJUST:
	if (gltx.linkId() == INVALID_ID)
	    edit = new CardAdjustment(main, gltx.id());
	else
	    edit = new CardTransfer(main, gltx.id());
	break;
    case DataObject::SHIFT:
	edit = new ShiftView(main, gltx.id());
	break;
    default:
	QMessageBox::critical(NULL, "Error", "Can't edit this transaction");
    }
    return edit;
}
