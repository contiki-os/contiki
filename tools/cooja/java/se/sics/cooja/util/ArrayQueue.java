/*
 * Copyright (c) 2010, BotBox AB. All rights reserved.
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
 * $Id: ArrayQueue.java,v 1.1 2010/09/09 19:51:53 nifi Exp $
 */

package se.sics.cooja.util;

import java.util.AbstractList;
import java.util.RandomAccess;

/**
 * The ArrayQueue class implements a simple queue using a rotating, resizable
 * array. Permits all elements, including <tt>null</tt>.
 *
 * The usage is basically the same as for ArrayList but this implementation
 * is optimized for adding last and removing first element while the ArrayList
 * shifts the data when removing first element.
 *
 * <p><strong>
 * Note that this implementation is not synchronized and if an ArrayQueue
 * instance is accessed by several threads concurrently, and at least one
 * thread modifies the queue, it must be synchronized externally.
 * </strong>
 *
 * @author  Joakim Eriksson (joakime@sics.se)
 * @author  Niclas Finne    (nfi@sics.se)
 */
public class ArrayQueue<E> extends AbstractList<E> implements RandomAccess, Cloneable, java.io.Serializable {

  private static final long serialVersionUID = 5791745982858131414L;

  private transient E[] queueData;
  private transient int first = 0;
  private transient int last = 0;
  private int size = 0;

  public ArrayQueue() {
    this(16);
  }

  public ArrayQueue(int initialCapacity) {
    if (initialCapacity < 0) {
      throw new IllegalArgumentException("illegal capacity: " +
                                         initialCapacity);
    }
    queueData = allocate(initialCapacity);
  }

  @SuppressWarnings("unchecked")
  private E[] allocate(int size) {
    return (E[]) new Object[size];
  }

  public void ensureCapacity(int minCapacity) {
    int capacity = queueData.length;
    if (capacity < minCapacity) {
//      int newCapacity = (capacity * 3) / 2 + 1;
      int newCapacity = capacity * 2;
      set(newCapacity < minCapacity ? minCapacity : newCapacity);
    }
    modCount++;
  }

  /**
   * Copies all data in the queue to a new data array of the specified
   * size (MUST be large enough) and replaces the old data array.
   */
  private void set(int newCapacity) {
    E[] newData = allocate(newCapacity);
    copy(newData);
    // The data is always in the beginning after allocating new data array
    first = 0;
    last = size;
    queueData = newData;
  }

  /**
   * Copies all data in the queue to the new data array (must be large enough!)
   */
  private void copy(Object[] newData) {
    if (first < last) {
      // No wrap ([...1,2,3,4...])
      System.arraycopy(queueData, first, newData, 0, size);
    } else if (size > 0) {
      // Wrapped queue ([4,5,6,......,1,2,3])
      int firstSize = queueData.length - first;
      // At least one element must exist since size > 0
      System.arraycopy(queueData, first, newData, 0, firstSize);
      System.arraycopy(queueData, 0, newData, firstSize, last);
    } else {
      // Empty queue i.e. do nothing
    }
  }

  public void trimToSize() {
    if (size < queueData.length) {
      // Make sure only the needed space is occupied
      set(size);
      modCount++;
    }
  }

  public int size() {
    return size;
  }

  public boolean isEmpty() {
    return size == 0;
  }

  public boolean contains(Object element) {
    return indexOf(element) >= 0;
  }

  public int indexOf(Object element) {
    return indexOf(element, 0);
  }

  public int indexOf(Object element, int index) {
    if ((index < 0) || (index >= size)) {
      return -1;
    }

    final int capacity = queueData.length;
    if (element == null) {
      for(; index < size; index++) {
        if (queueData[(first + index) % capacity] == null) {
          return index;
        }
      }

    } else {
      for(; index < size; index++) {
        if (element.equals(queueData[(first + index) % capacity])) {
          return index;
        }
      }
    }
    return -1;
  }

  public int lastIndexOf(Object element) {
    final int capacity = queueData.length;
    if (element == null) {
      for(int index = size - 1; index >= 0; index--) {
        if (queueData[(first + index) % capacity] == null) {
          return index;
        }
      }

    } else {
      for(int index = size - 1; index >= 0; index--) {
        if (element.equals(queueData[(first + index) % capacity])) {
          return index;
        }
      }
    }
    return -1;
  }

  public E get(int index) {
    // getPos() will ensure that the index is valid
    return queueData[getPos(index)];
  }

  public E set(int index, E element) {
    int pos = getPos(index);
    E oldValue = queueData[pos];
    queueData[pos] = element;
    return oldValue;
  }

  public void add(int index, E element) {
    // Make sure there is space for the new element.
    ensureCapacity(size + 1);
    if (index == size) {
      // Add to the end of the queue
      queueData[last] = element;
      last = (last + 1) % queueData.length;
      size++;
    } else {
      // This will make sure the index is valid (0 <= index < size)
      index = getPos(index);

      if (index == first) {
        // Add to the beginning of the queue
        if (first > 0) {
          first--;
        } else {
          // first == 0 => move first to the end of the data array
          first = queueData.length - 1;
        }
        index = first;
      } else if (index < last) {
        // Non wrapped queue or index is in the lower part of the data array
        System.arraycopy(queueData, index, queueData, index + 1, last - index);
        last = (last + 1) % queueData.length;
      } else {
        // Wrapped queue (index in the end of the data array)
        // first > 0 because last < first (there is at least one free position)
        System.arraycopy(queueData, first, queueData, first - 1,
                         index - first);
        index--;
        first--;
      }
      queueData[index] = element;
      size++;
    }
  }

  public E remove(int index) {
    E value;
    // This method also checks that the index is valid
    index = getPos(index);

    value = queueData[index];
    if (index == first) {
      // Remove first element. The most common case for a queue.
      queueData[first] = null;
      first = (first + 1) % queueData.length;

    } else if (index < last) {
      // Non wrapped queue or index is in the lower part of the data array.
      last--;
      if (index < last) {
        // An element in the middle is removed
        System.arraycopy(queueData, index + 1, queueData, index, last - index);
      }
      queueData[last] = null;

    } else if ((last == 0) && (index == queueData.length - 1)) {
      // Minor optimization: no elements at the beginning of the data
      // array and the removed index is last in the data array.
      queueData[index] = null;
      last = queueData.length - 1;

    } else {
      // Wrapped queue (index in the end of the data array)
      System.arraycopy(queueData, first, queueData, first + 1, index - first);
      queueData[first++] = null;
    }
    size--;
    modCount++;
    return value;
  }

  public boolean remove(Object element) {
    int index = indexOf(element);
    if (index >= 0) {
      remove(index);
      return true;
    }
    return false;
  }

  public void clear() {
    final int capacity = queueData.length;
    for (int i = 0; i < size; i++) {
      queueData[(first + i) % capacity] = null;
    }
    first = last = size = 0;
    modCount++;
  }

  /**
   * Returns a shallow copy of this queue (the elements themselves are not
   * copied).
   */
  @SuppressWarnings("unchecked")
  public ArrayQueue<E> clone() {
    try {
      ArrayQueue<E> v = (ArrayQueue<E>) super.clone();
      // The set method will always create a new array which gives the
      // cloned queue its own copy of the data array.
      v.set(v.size);
      v.modCount = 0;
      return v;
    } catch (CloneNotSupportedException e) {
      // Should never happen
      throw new InternalError();
    }
  }

  public Object[] toArray() {
    Object[] array = new Object[size];
    copy(array);
    return array;
  }

  @SuppressWarnings("unchecked")
  public <T> T[] toArray(T[] array) {
    if (array.length < size) {
      array = (T[]) java.lang.reflect
        .Array.newInstance(array.getClass().getComponentType(), size);
    }
    copy(array);
    if (array.length > size) {
      array[size] = null;
    }
    return array;
  }

  private int getPos(int index) {
    if ((index >= 0) && (index < size)) {
      return (first + index) % queueData.length;
    }
    throw new IndexOutOfBoundsException("Index: " + index + ", Size: " + size);
  }

  public String toString() {
    StringBuilder sb = new StringBuilder();
    sb.append(getClass().getName()).append('[')
      .append(first).append(',').append(last)
      .append(',').append(size)
      .append(",[");
    if (queueData.length < 32) {
      for (int i = 0, n = queueData.length; i < n; i++) {
        if (i > 0) sb.append(',');
        sb.append(queueData[i]);
      }
    } else {
      sb.append("...");
    }
    sb.append("]]");
    return sb.toString();
  }

  // -------------------------------------------------------------------
  //  Serialization
  // -------------------------------------------------------------------

  private void writeObject(java.io.ObjectOutputStream out)
    throws java.io.IOException
  {
    int queueLen = queueData.length;
    out.defaultWriteObject();
    out.writeInt(queueLen);
    for (int i = 0, index = first; i < size; i++) {
      out.writeObject(queueData[index]);
      index = (index + 1) % queueLen;
    }
  }

  @SuppressWarnings("unchecked")
  private void readObject(java.io.ObjectInputStream in)
    throws java.io.IOException, ClassNotFoundException
  {
    in.defaultReadObject();
    queueData = (E[]) new Object[in.readInt()];
    for (int i = 0; i < size; i++) {
      queueData[i] = (E) in.readObject();
    }
    first = 0;
    last = size;
  }

} // ArrayQueue
