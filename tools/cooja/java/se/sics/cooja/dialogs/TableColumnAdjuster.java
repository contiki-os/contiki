/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * -----------------------------------------------------------------
 *
 * Author  : Niclas Finne
 * Created : 2009-05-20
 * Updated : $Date: 2009/06/12 14:12:59 $
 *           $Revision: 1.1 $
 */

package se.sics.cooja.dialogs;
import java.awt.Component;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Arrays;

import javax.swing.JTable;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;

/**
 * This code was originally based on a Java tip from Rob Camick at
 * Java Tips Weblog, http://tips4java.wordpress.com.
 */
public class TableColumnAdjuster implements PropertyChangeListener, TableModelListener {

  private final JTable table;
  private int spacing;
  private boolean isColumnHeaderIncluded;
  private boolean isColumnDataIncluded;
  private boolean isOnlyAdjustLarger;
  private boolean isDynamicAdjustment;

  /*
   *  Specify the table and use default spacing
   */
  public TableColumnAdjuster(JTable table) {
    this(table, 6);
  }

  private boolean[] adjustColumns;

  /*
   *  Specify the table and spacing
   */
  public TableColumnAdjuster(JTable table, int spacing) {
    this.table = table;
    this.spacing = spacing;
    TableColumnModel tcm = table.getColumnModel();
    adjustColumns = new boolean[tcm.getColumnCount()];
    Arrays.fill(adjustColumns, true);

    setColumnHeaderIncluded(true);
    setColumnDataIncluded(true);
    setOnlyAdjustLarger(true);
    setDynamicAdjustment(false);
    
  }

  public void packColumns() {
    TableColumnModel tcm = table.getColumnModel();
    for (int i = 0, n = tcm.getColumnCount(); i < n; i++) {
      packColumn(i);
    }
  }

  public void packColumn(final int column) {
    adjustColumn(column, false);
  }

  /*
   *  Adjust the widths of all the columns in the table
   */
  public void adjustColumns() {
    TableColumnModel tcm = table.getColumnModel();
    for (int i = 0, n = tcm.getColumnCount(); i < n; i++) {
      if (adjustColumns[i]) {
        adjustColumn(i, isOnlyAdjustLarger);
      }
    }
  }

  public void setAdjustColumn(int i, boolean adjust) {
    adjustColumns[i] = adjust;
  }
  
  /*
   *  Adjust the width of the specified column in the table
   */
  public void adjustColumn(int column) {
    adjustColumn(column, isOnlyAdjustLarger);
  }

  private void adjustColumn(int column, boolean onlyAdjustLarger) {
    if (!adjustColumns[column]) {
      return;
    }
    int viewColumn = table.convertColumnIndexToView(column);
    if (viewColumn < 0) {
      return;
    }
    TableColumn tableColumn = table.getColumnModel().getColumn(viewColumn);
    if (! tableColumn.getResizable()) {
      return;
    }

    int columnHeaderWidth = getColumnHeaderWidth(tableColumn, column);
    int columnDataWidth   = getColumnDataWidth(tableColumn, column);
    int preferredWidth    = Math.max(columnHeaderWidth, columnDataWidth);
    updateTableColumn(tableColumn, preferredWidth, onlyAdjustLarger);
  }

  /*
   *  Calculate the width based on the column name
   */
  private int getColumnHeaderWidth(TableColumn tableColumn, int column) {
    if (! isColumnHeaderIncluded) return 0;

    Object value = tableColumn.getHeaderValue();
    TableCellRenderer renderer = tableColumn.getHeaderRenderer();

    if (renderer == null) {
      renderer = table.getTableHeader().getDefaultRenderer();
    }

    Component c = renderer.getTableCellRendererComponent(table, value, false, false, -1, column);
    return c.getPreferredSize().width;
  }

  /*
   *  Calculate the width based on the widest cell renderer for the
   *  given column.
   */
  private int getColumnDataWidth(TableColumn tableColumn, int column) {
    if (! isColumnDataIncluded) return 0;

    int preferredWidth = 0;
    int maxWidth = tableColumn.getMaxWidth();

    for (int row = 0, n = table.getRowCount(); row < n; row++) {
      preferredWidth = Math.max(preferredWidth, getCellDataWidth(row, column));

      //  We've exceeded the maximum width, no need to check other rows
      if (preferredWidth >= maxWidth) {
        break;
      }
    }

    return preferredWidth;
  }

  /*
   *  Get the preferred width for the specified cell
   */
  private int getCellDataWidth(int row, int column) {
    //  Invoke the renderer for the cell to calculate the preferred width

    TableCellRenderer cellRenderer = table.getCellRenderer(row, column);
    Object value = table.getModel().getValueAt(row, column);
    Component c = cellRenderer.getTableCellRendererComponent(table, value, false, false, row, column);
    int width = c.getPreferredSize().width + table.getIntercellSpacing().width;
    return width;
  }

  /*
   *  Update the TableColumn with the newly calculated width
   */
  private void updateTableColumn(TableColumn tableColumn, int width, boolean onlyAdjustLarger) {
    int currentWidth = tableColumn.getWidth();
    width += spacing;

    // Don't shrink the column width if onlyAdjustLarger is set
    if (width != currentWidth && (!onlyAdjustLarger || width > currentWidth)) {
      table.getTableHeader().setResizingColumn(tableColumn);
      tableColumn.setWidth(width);
      tableColumn.setPreferredWidth(width);
    }
  }

  private void adjustColumnsForNewRows(int firstRow, int lastRow) {
    TableColumnModel tcm = table.getColumnModel();
    for (int column = 0, n = tcm.getColumnCount(); column < n; column++) {
      if (!adjustColumns[column]) {
        continue;
      }
      int viewColumn = table.convertColumnIndexToView(column);
      if (viewColumn < 0) {
        continue;
      }
      TableColumn tableColumn = tcm.getColumn(viewColumn);
      if (! tableColumn.getResizable()) {
        continue;
      }
      // Find max width for the new rows (only adjust if wider)
      int width = 0;
      for (int row = firstRow; row <= lastRow; row++) {
        int w = getCellDataWidth(row, column);
        if (w > width) {
          width = w;
        }
      }
      updateTableColumn(tableColumn, width, true);
    }
  }

  /*
   *  Indicates whether to include the header in the width calculation
   */
  public void setColumnHeaderIncluded(boolean isColumnHeaderIncluded) {
    this.isColumnHeaderIncluded = isColumnHeaderIncluded;
  }

  /*
   *  Indicates whether to include the model data in the width calculation
   */
  public void setColumnDataIncluded(boolean isColumnDataIncluded) {
    this.isColumnDataIncluded = isColumnDataIncluded;
  }

  /*
   *  Indicates whether columns can only be increased in size
   */
  public void setOnlyAdjustLarger(boolean isOnlyAdjustLarger) {
    this.isOnlyAdjustLarger = isOnlyAdjustLarger;
  }

  /*
   *  Indicate whether changes to the model should cause the width to be
   *  dynamically recalculated.
   */
  public void setDynamicAdjustment(boolean isDynamicAdjustment) {
    if (this.isDynamicAdjustment != isDynamicAdjustment) {
      this.isDynamicAdjustment = isDynamicAdjustment;
      if (isDynamicAdjustment) {
        table.addPropertyChangeListener(this);
        table.getModel().addTableModelListener(this);
      } else {
        table.removePropertyChangeListener(this);
        table.getModel().removeTableModelListener(this);
      }
    }
  }

  //
  //  Implement the PropertyChangeListener
  //
  public void propertyChange(PropertyChangeEvent e) {
    //  When the TableModel changes we need to update the listeners
    //  and column widths

    if ("model".equals(e.getPropertyName())) {
      if (this.isDynamicAdjustment) {
        TableModel model = (TableModel)e.getOldValue();
        model.removeTableModelListener(this);

        model = (TableModel)e.getNewValue();
        model.addTableModelListener(this);
      }
      adjustColumns();
    }
  }

  //
  //  Implement the TableModelListener
  //
  public void tableChanged(final TableModelEvent e) {
    if (e.getType() == TableModelEvent.INSERT) {
      adjustColumnsForNewRows(e.getFirstRow(), e.getLastRow());

    } else if (e.getType() == TableModelEvent.UPDATE) {
      int column = e.getColumn();
      int lastRow = e.getLastRow();
      // Last row might be set higher than the row count if all rows have
      // been updated
      if (lastRow >= table.getRowCount()) {
        lastRow = table.getRowCount() - 1;
      }
      if (column == TableModelEvent.ALL_COLUMNS) {
        // All columns have been updated
        if (isOnlyAdjustLarger) {
          int firstRow = e.getFirstRow();
          if (firstRow >= 0) {
            // Handle the rows as new rows since they should only increase
            // width as needed
            adjustColumnsForNewRows(firstRow, lastRow);
          }
        } else {
          // Could be an increase or decrease so check all rows, all columns
          adjustColumns();
        }

      } else if (isOnlyAdjustLarger) {
        //  Only need to worry about an increase in width for these cells
        if (!adjustColumns[column]) {
          return;
        }
        int viewColumn = table.convertColumnIndexToView(column);
        if (viewColumn < 0) {
          // Column is not visible
        } else {
          TableColumn tableColumn = table.getColumnModel().getColumn(viewColumn);
          if (tableColumn.getResizable()) {
            int firstRow = e.getFirstRow();
            int width = 0;
            if (firstRow < 0) {
              // Header changed
              width = getColumnHeaderWidth(tableColumn, column);
            } else {
              for (int row = e.getFirstRow(); row <= lastRow; row++) {
                int w = getCellDataWidth(row, column);
                if (w > width) {
                  width = w;
                }
              }
            }
            updateTableColumn(tableColumn, width, true);
          }
        }
      } else {
        // Could be an increase or decrease so check all rows
        adjustColumn(column, false);
      }

    } else {
      // Some rows have been deleted.
      if (!isOnlyAdjustLarger) {
        adjustColumns();
      }
    }
  }

}
