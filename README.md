cashflow
========
Zero-sum budgeting tool for the desktop

Version 0.23.1

Quick how-to
------------
F2 edits!

Steps:

1. add a period and edit the name
2. register previously unregistered items to fill in. In flows are income (positive to the balance) and out flows are expenses (negative to the balance)
3. edit the registered item's note, budget or actual fields
4. repeat Steps 2 and 3 until you balance the period budget and actuals to zero (ie. Ins = Outs).


This version is in old UI application style, menu based. Its mainly written with keyboard enthusiasts in mind, so no context menus either (not that those are new.) You can click on fields to edit them, although only the intended ones will allow editing.

The register shows the metrics, or live data, the budget and actuals. Press F2 to edit registered budget or actual amounts or to alter or enter a new value for periods in the period docked window.

Special Thanks
--------------
* [Qt Project](http://qt-project.org/)
* [Sqlite](https://sqlite.org/)
* [greenpit.com](http://www.greepit.com/open-source-icons-gcons/)

Build Dependencies
------------------
Qt 4.7.3 or later stable version of Qt 4

Sqlite version 3

Usage
-----
The period metrics window shows the balances (in minus out) for budgeted and actual spent amounts, plus a diff of those two. These should always sum to zero to follow a zero-sum budget.

Why, you ask?

Alert! Nerdy explanation pending!
  
```c++
void ZeroBasedBudgettingPhilosophyExplanation()
{
  bool blahBlahBlah_IKnowThisAlready = brainScan(you);

  // skip this if you know this concept already or 
  // (or if you find it corny)

  while (blahBlahBlah_IKnowThisAlready == false) {

    In most cases having zero money is worse than having some money, but 
    better than owing money (much better.)

    In this case having zero is a very good thing! I put this comment on 
    its own line because this point is so important. Oh, whoops, no I 
    didn't ...

    In this case having zero is a very good thing!

    (There, now that's better.)

    The reason having a zero-sum is good is because any difference in your 
    income and expenses shows you either are spending more than you are 
    taking in (a mathmatical impossibility) or you are holding back income 
    for that rainy-day shopping spree. Neither are necessary. Giving every 
    penny of income and expense a name will free you to direct the flow of 
    your cash (in and out.) So if you want to spend that extra income on a 
    no-holds barred shopping bonanza, then just do it, but record that 
    money as an out for Category "Personal" and Item "Shopping Spree" or 
    more likely "Health" and "Therapy" and make sure you have the income to 
    support it.

    At least, this way you will know what's left over to put towards rent, 
    date-night or the other important things in life.

    (No one is judging you ... or this program won't anyway. Not until I 
    get that Neural-network AI Spending-Nanny feature working :)

    A zero-sum budget is made to assist meeting your financial goals (or 
    the financial portions of your goals.) When you save for something, 
    that will fall under "Outflow" and "Savings" and an item naming the 
    reason. When you spend some of that savings, it will come from "Inflow" 
    and "Savings" from a similarly named item as what you saved to.
    
    Pick a primary savings goal per period (month, year, etc.) All of your 
    excess after needs and wants can go towards that savings goal.
    
    That way, if you overspend, it will come directly to your attention 
    because you will have to take it out of the money you intended for your 
    primary savings goal. Those occurances will teach you to be more 
    disciplined.
    
    With zero-sum budgeting, you will eventually set the budget and actual 
    each item to what you actually spent on that item. In other words, for 
    budget is never less than actual and should be equal once the period 
    has expired and all budgetted money for each item is spent. (The 
    difference value should show zero, when the period is up.)
    
    One last thing. A budget isn't for accounting or keeping track of your 
    account balances. Use other personal finance tools for that.

    // do a internet search for "zero sum budget how to" to learn more

    blahBlahBlah_IKnowThisAlready = brainScan(you);
  }
}
```

The selecting a period will show the metrics of that period inside the flow metrics window, if any.

You can also edit the period name in the period view, if you wish. Press F2 to edit.

The flow metrics window shows the in totals and out totals for budgeted and actually spent. Here the in's and out's should always be equal, their difference being zero, to maintain a zero-base. (See above for why.)

The selecting a flow will show the metrics of that flow inside the category metrics window, if any.

The flow name is not editable.

The category metrics window shows the budgeted and actuals for the in or out flow categories their mapped flows.

The selecting a category will show the metrics of that category inside the item metrics window, if any.

The category name is not editable, here. You must go into the Manage Categories window (via the button on the bottom.)

The register window shows the budgeted and actuals for the in or out flow items, driven by their mapped categories and thier mapped flows.

The default is for them to both be zero. These values get summed up to the higher windows: category, flow and period.

Press F2 to edit either the budget and actual metrics in the register window. Only the budget and actual can be edited. The difference column auto updates with the formula (budget - actual).

What works
----------
The metrics are showing through the views, also creating the difference column.

The seperate views read the register data and populate themselves based on the period that it contains. As the different views are selected, it filters the next level down. The filter control pattern goes:
```
Period ----------------------> Register
   |                              ^
   v---> Flow -> Category -> Item |
```

The categorys' names and flows can be changed, added and deleted. The items' names and categories can be changed, added and deleted.

Items can be registered and unregistered for each period, depending on whether you want to use it for that period, or not.

What's broken in this version
-----------------------------
Saved files loose their undo-redo focus and look as if they are "redone" all the way. When saving a file, it should remove the extra redos.

```
BadWindow error is occuring again. Its likely the progress bar is overflowing.
>>>
$ X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 20 (X_GetProperty)
  Resource id:  0x1c1daa0
X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 15 (X_QueryTree)
  Resource id:  0x1c1daa0
X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 40 (X_TranslateCoords)
  Resource id:  0x1c1daa0
<<<
```

What's left to do
-----------------
Find another key other than F2 for edit, such as Enter.

Allow customizing of styles and colors.

Allow saving of preferences on close.

Add a filter on each window for the editable views.

Hide/show certain categories. Hidden metrics will not propogate up.

Hide/show certain items. Hidden metrics will not propogate up.

Add savings funds to both in and out flow savings category.

Add a cancel button that will revert the changes in the managed windows.

Allow deleting used categories if another existing category is assigned to receive item mappings.

Allow deleting used item if another existing item is assigned to receive item mappings.

Allow saving a template of category->item map.

Allow loading a template (category->item map) from another month (each month can have its own template)?

Create a tree view interface

Use one tree view for all

Try QML for a fresher looking interface.

Display reports.

Allow custom report queries.

Incorporate a report design tool.

Move to Qt 5

What's done
-----------
[Done in v0.23 on 20140223]
Reconfigured this file in Markdown format.

Added changes for staging to github.

[Done/Fixed in v0.23 on 20140219]
Added the png files to the repository and their qrc resource file. I added the qrc_*.cpp file to the Hg ignore file.

Changed the setEditTriggers for the model views to be editable via clicking as well as the F2 Edit key.

Added short how-to at beginning of README.txt

[Done in v0.22 on 20140217]
Changed out some icons and updated this file to have a "Special Thanks" section.

[Done in v0.22 on 20140217]
Changed out some icons and updated this file to have a "Special Thanks" section.

[Done in v0.22 on 20140216]
I updated the copyright and license information. I opened the code using the Apache v2.0 license and the Open Database License v1.0.

[Done in v0.21 on 20130818]
Removed any existing real data from the repositories.

[Fixed in v0.21 on 20130616]
I changed some of the wording on the Save As and Clone As actions. I also changed the Clone As action to Backup As.

[Fixed in v0.21 on 20130308]
When undoing a cloned period, the extra steps of undo are still available to remove the individual items that got registered to that period. This breaks the flow of the application when one has to undo each registered item individually especially if they can't see them beging removed because the period is already gone.

[Fixed in v0.21 on 20130304]
The last fix to the clone period feature caused all of the copied register items, not to insert because the period was not inserted yet and the program is using a foriegn key of periodId in the register. (This was strangely working after the first attempt.)

Now the clone function is submitting with changes to the new id and name.

[Fixed in v0.21 on 20130303]
When I wrote the clone period feature, I used the insert trigger for the register metrics view, that simply added a empty space for note.
Cloning required copying the notes over too, so I changed this trigger to also copy the new.note field value. This messed up the registerItem() function that manually does an insert. The solution was to also add the empty space while manually setting the budget, actual, etc.

[Fixed in v0.21 on 20130303]
The program doesn't ask to save when exit is called with an unsaved file with changes loaded.

[Fixed in v0.21 on 20130224]
Clone a period.

[Closed in v0.21 on 20130202]
The following error occurs when undoing all the way:
"QAbstractItemModel::endRemoveRows:  Invalid index ( 1 , 1 ) in model SqlTableModel."

I was not able to reproduce this issue.

[Closed in v0.21 on 20130202]
Unregistering items doesn't cause a redraw of the period view or change its total back to zero.

I was not able to reproduce this issue.

[Fixed in v0.21 on 20130129]
Adds in the managed item window do not update the main forms undo action. It requires doing two (and redoing) before it will signal that an undo is available.

The fix for this was made by calling a setter for a bool on the main form with a change signal from the manage map form (either category or item). Immediately after closing the form, the bool is read and the show change function is ran, then the bool is reset to false. This seems to work for both forms. One downside to this is the * doesn't show up immediately, but rather when the map form is exited. Undo/redo works too.

[Fixed in v0.21 on 20130124]
Calling New file action now clears the category table.

[Fixed in v0.21 on 20130121]
Changes in the managed category window does not update the main forms title bar as having unsaved changes.

The mappingChanged signal is now occuring for the add, edit and delete of a category.

[Done in v0.21 on 20130120]
Allow reverting to prior save.

[Done in v0.21 on 20130118]
Allow sorting the column one-at-a-time on the main form.

[Done, completely in v0.20 on 20130117]
Implement an undo feature.
Implement an redo feature.

The undo and redo works with adding, changing and removing all editable views including the category and item mappings. The undo/redo log is kept with the saved file. The modified indicator works with the mapping changes and undo/redo.

[Done, partially in v0.20 on 20130116]
Worked more on the undo/redo feature so that items can be registered and have their values changed. New changes that occur in the middle of the undo/redo list eliminates the rest of the redos in the logUndoRedo.

[Done, partially in v0.20 on 20130116]
Implemented undo/redo feature that effects adding and removing periods and registered items.

[Fixed in v0.19 on 20130110]
Its was not possible to edit or delete an item from the map view, neither in nor out. This now works. The problem was the QSqlRelationalTableModel's relational column was not saving back the categoryId because it was showing the category name instead. I believe this was not working because model's table was a Sqlite SQL View. However it did have instead of triggers for CUD operations, so its kind of a mystery.

The fix was made by showing the category/flow on another view in the same dialog and having it selected while adding an item or mapping the item to a new category. Deleting now works too.

[Fixed in v0.19 on 20130101]
Changes in the managed item window now updates the main forms title bar as having unsaved changes.

[Done in v0.19 on 20121217]
Implement a "Close" file action.

[Fix in v0.19 on 20121217]
The open action is not adding the opened database to the recently opened file list.

[Fix in v0.19 on 20121217]
The "New" action on the MainForm causes an non-terminal error:
X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 20 (X_GetProperty)
  Resource id:  0x1c18f40
X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 15 (X_QueryTree)
  Resource id:  0x1c18f40
X Error: BadWindow (invalid Window parameter) 3
  Major opcode: 40 (X_TranslateCoords)
  Resource id:  0x1c18f40

This ended up being the progress bar was being pushed to a value higher than it contained in the next to last case.
  progress.setValue(++progressCounter);
To fix this I just added one to the limit.

[Fix in v0.19 on 20121217]
Fix issue of needing to force a reset on the QMainWindow subclass when a new database is loaded. This is because the widgets on the window are still using the database when it is removed. The error is,
  "QSqlDatabasePrivate::removeDatabase: connection 'qt_sql_default_connection' is still in use, all queries will cease to work."

I fixed the above issue by removing the reset of the MainForm in the Application::open method and removing the removeDatabase/addDatabase step before/after copying the opened file over the working file in Data::open method.

This caused its own issue. Since I was no longer reseting the MainForm, I needed to release the prior versions of the MainForm's contents since they were sticking around while the new ones were drawn. The open method in the MainForm is now running similar functions to the new method. To remedy this, I created a function to remove the form objects including the menus. This caused a "Application asked to unregister timer ..." error each time I reopened and eventually lead to a SEGFAULT. The fix was to make sure I was deleting the topmost (the parents) of the widgets and to replace deleting the menus with the QMenuBar clear() method.

I can probably now change MainForm from a QScopePointer to a QMainWindow in Application class.

[Done in v0.19 on 20121210]
Some minor testing has been done with qss stylesheets and button images. I have not decided what to do with it yet. I will probably use it for further testing of switching the look and feel via preferences (also to be created later.)

[Fix in v0.19 on 20121210]
The In flow values were counted as negative in the period view. This has been fixed by joining on the flow name = 'In' versus using 1 as the primary key (the old pk value for inflow). This could potentially be a problem if 'In' is entered into the flow table again under a different key.

[Done in v0.19 on 20121207]
Allow saving of recent file list on close and reload upon opening.

[Fix in v0.19 on 20121207]
Versioning Fixed.
Note!!! Because I wasn't aware of how the versioning via tags was working, all of the Fix and Done bullet points below were actually one version behind. I was creating the tag, then saying the next Fix was of that version. I now understand that I need to bump my version (in the Help|About and in the README.txt) ahead of my tag in Mercurial. For prosperity's sake, I will only change this copy of the README.txt with the corrected versions and those going forward since they are going to get commited with the correct version.

[Fix in v0.19 on 20121207]
The recent files list has been fixed to stay in place even when opening a file. This was a problem because opening a file resets the QMainWindow subclass where the recent file list was stored. Its now stored in the Application object.

[Done in v0.19 on 20121207]
Now it is able to open recent files. I added this feature and it loads a new filename into the list when the file is saved. There is an issue with the list clearing upon opening a file.

[Done in v0.19 on 20121206]
Use UUID's instead of integers for auto-increment primary key. Rewrote the managed category and item mapping forms.

When updating register values and propogating up the changes, make sure summary views do not lose their selections. Currently they are starting over at the top one. Have them stay the same or return to the ones they were on when the change is made.

[Removed To-Do in v0.18 on 20121203]
Update the status bar as things change.

[Fixed in v0.18 on 20121203]
The views will now expand the column names to their full sizes.

[Fixed in v0.18 on 20121203]
Fixed error where registering an item was causing the flow or category (whichever was raised and selected) to be unselected. This was causing the registered item view to refresh upon registering.

[Fixed in v0.18 on 20121203]
Fixed error where unregistering an item was causing the flow or category (whichever was raised and selected) to be unselected. This was causing the registered item view to refresh upon unregistering.

[Fixed in v0.18 on 20121203]
Fixed error where editing a record was causing the period to be deselected and therefore would kick out an error when a unused item was registered.

[Done in v0.18 on 20121203]
Add a notes field for comments.

[Fixed in v0.18 on 20121203]
The period view was not updating once the registered items were unregistered. I fixed this by overriding the QSqlTableModel::removeRow() method and emitting the dataSubmitted signal I created for updating the views, earlier.

[Done in v0.18 on 20121129]
Allow deleting only unregistered items.

[Done in v0.18 on 20121128]
Allow deleting only unused categories.

[Done in v0.18 on 20121128]
Allow unregistering items with 0 for budget and actual values without warning dialog, otherwise warn.

[Fixed in v0.18 on 20121127]
Don't show the CloneAs name as the last saved name in the properties.

[Fixed in v0.17 on 20121125]
Not propogating values upward in the MainForm from a lower view to higher automatically. Propogation only occurs when selecting a different period, if there is one.

Period metrics aren't propogated at all until app reboot.

[Fixed in v0.17 on 20121122]
Make sure the CloneAs action does not change the current titlebar text or saved file name.

[Fixed in v0.17 on 20121122]
Fix the opened files so they are read-write, instead of read only. This was done by wrapping the MainForm in a QScopedPointer and making that a member of the Application class. When the open method is called on the MainForm, it will first run close() on itself, then run the Application's open method which resets the pointer object, thereby creating a new form. This is probably not the best way to do this, but will work for now.

[Fixed in v0.17 on 20121118]
Be able to create a new cashflow file.

Be able to save file.

Be able to open a saved file.

Put currently loaded file name in the title bar.

I added the Data class as a member of the Application class, itself a subclass of QApplication.

I cleaned up the unregister items function and removed the SQL, by adding a SQL trigger to delete through the SQL view. Unfortunately, I will need to find a new way to do a "are you sure" prompt.

[Fixed in v0.17 on 20121107]
Move the data functions from main.cpp into its own functions and files.

[Fixed in v0.17 on 20121014]
When changing unregisteredItemView to unusedMetricsView in the MainForm code, I forgot to change it in the database creation code. These changes fix that.

[Done in v0.17 on 20121014]
Move docked windows to top and stack them. Remove the view label.

[Abandoned in v0.17 on 20121014]
Add an Item view seperate from the register for filtering. I did go with the changes made to give the register its own Qt View. I deleted all item Qt view constructs because it was redundant. If its desired to filter on item (which is what I was trying to do) it will be better to do it in the register view. Also it will be necessary to have all periods be shown, if a item filter is to be effective.

[Done in v0.16 on 20121013]
I changed the "register item" action to put focus back down to the same or a prior row on the unregistered view after finished registering.

[Fixed in v0.16 on 20121013]
Can now insert and update periods.

[Done in v0.16 on 20121008]
Moved the application version to a constant.

Use single cell select for Item View and use whole line select for all others.

[Done in v0.16 on 20121008]
Create more/better menus. I added Edit and View menus. I moved the thing effecting the registry (periods/items) to the Edit Menu and left the mapping actions in the File menu. To the View menu, I added show/hide toggle actions for each docked view plus the unregistered items panel.

[Done in v0.16 on 20121007]
Hide the actions in main menu items and remove the buttons associated with them.

[Done in v0.16 on 20121003]
Display the numeric values right aligned, the text values left aligned. This was done by adding a subclass of QSqlTableModel and capture/change the data() call of TextAlignmentRole to return Qt::RightAlign if certain numeric field names are the focus of the model index parameter.

[Done in v0.15 on 20120823]
I moved the docked view widgets to show on the left hand area.

[Done in v0.15 on 20120722]
I removed the filter views (Period, Flow and Category) from the splitter and added them back as docks.

Show period, flow and category in the item view. I also added the foreign key fields to each view to help identify like named categories or items.

[Done in v0.14 on 20120705]
Add pretty column names to the MainForm views.

[Done in v0.14 on 20120704]
Try moving the registered item view to the right of all of the other views in the MainForm.

[Fixed in v0.14 on 20120704]
I changed the Qt views to always show two decimal places for the numeric fields.

[Fixed in v0.14 on 20120703]
When period's budget balance is zero, it is being shown as -9.??e-13. It should show 0.

Allow renaming a period. (Wasn't a real bug.)

[Fixed in v0.14 on 20120701]
I commented out the section that creates a insert SQL and left the Qt way of doing it.

[Done in v0.13 on 20120630]
I added a "are you sure?" dialog when the period is deleted on the MainForm.

[Fixed in v0.13 on 20120624]
Have selection of Flow window in MainForm set restrictions on Item window just like the Category window does. I cleaned up the code for the "update view" methods for the model restrictions and view labels contents. Now the user should be able to start the item view filter over (clear out the flow and category restrictions) by clicking another period (same goes for clearing out category restriction by clicking antother flow).

Deleting Periods should delete all entries in the Register for that period (ie cascading delete). I had to add a flag for SQLite3 in the main.cpp file to activate the foreign keys and add "on delete cascade" clause to the create table statement for the register table. I also added them for the item and category tables.

Preexisting periods are not deleting or inserting. Add SQL trigger to the views for each. Change the period metrics view to do a left outer join so that all periods will show up even if they are not in the register.

[Fixed in v0.13 on 20120621]
Allow specification of period for unregistering (delete) active items from the MainForm's item metrics window (from the register). This way the unregistered items view will always show whats not currently being used in each period.

[Done in v0.13 on 20120621]
Allow new items to be registered (added) to the MainForm's item metrics window (into the register). There is an issue with using the "in-Qt-View" insert with the items metric window in that we only want to be able to allow unused items to be inserted, but the list view widget create by the setRelation fuction will only show whatever table the model has a relation set to. If it is the set of unused items, then the relation will restrict out all items and it will show no entries in that metrics window. Therefore it will look like nothing is in the register.

Have selection of Period window in MainForm set restrictions on Item window just like the Category window does

[Done in v0.13 on 20120617]
Delete active items from the MainForm's item metrics window (from the register).

[Done in v0.13 on 20120610]
Clean up the main forms button collection at the bottom. Perhaps move the buttons to the view they are used with.

[Fixed in v0.13 on 20120610]
Errors in stderr when opening and closing the Manage Items window:
>>>
Application asked to unregister timer 0x38000008 which is not registered in this thread. Fix application.
Application asked to unregister timer 0x3a000002 which is not registered in this thread. Fix application.
<<<

This error was cause by the OnManualSubmit flag for the in and out item models:
inItemModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
outItemModel->setEditStrategy(QSqlTableModel::OnManualSubmit);

The fix was to change it to OnRowChange. This was the original setting, but I had changed it to test being able to cancel out of adding or deleting mappings. With the on manual submit, I had it where the user could cancel adds but not deletes. I figure that canceling deletes are what counts, and even with manual submit on, I had to do immediate submits after the delete to the row or it would still show until the window was closed. That being the I decided there was no use in keeping the manual submit turned on.

Here is what they were changed to:
inItemModel->setEditStrategy(QSqlTableModel::OnRowChange);
outItemModel->setEditStrategy(QSqlTableModel::OnRowChange);

[Done in v0.13 on 20120603]
Brief explanation of metrics window including goofy zbb explanaiton rant no one asked for.

[Fixed in v0.12 on 20120602]
The manage items window does not show flow name of parent category so category name is possibly ambiguous since it can exist in both in and out flow.

[Done in v0.12 on 20120602]
Find another UI format to add and remove items (maybe categories, optional)

I changed the manage categories and manage items windows so that they would look similar with two QTableViews in each with In or Out.

The drop down for the relational category name was pulling all of the categories, not just the in or out flow, exclusivly. I changed the setRelation to join to a category map view for each in and out.

[Done in v0.11 on 20120515]
Allow adding new items

[Done in v0.11 on 20120515]
Allow deleting items

[Done in v0.11 on 20120512]
Allow adding new categories

[Done in v0.11 on 20120512]
Allow deleting categories

[Done in v0.11 on 20120512]
Remove the register view and allow changes in the Item view. The item view (while the flow, category and period views are present) can house the interface for changes, making the register view redundant.

[Fixed in v0.10 on 20120505]
Cannot change period name.
