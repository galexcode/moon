//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2009 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

using System.Collections.Specialized;

namespace System.Windows.Controls {

	public sealed partial class ItemCollection : PresentationFrameworkCollection<object> {

		bool readOnly;

		internal void SetReadOnly ()
		{
			readOnly = true;
		}

		// Note: Parameter handling is different from other PresentationFrameworkCollection<T> types
		// but it may not be limited to this
		internal override bool NullCheck (NotifyCollectionChangedAction action, object value)
		{
			if (value == null)
				throw new ArgumentException ();
			return false;
		}

		// current code cannot compare System.Object in unmanaged code (as it compare pointers and we
		// supply GC handles) so we do the comparison in managed code (FIXME: slow but working)
		internal override int IndexOfImpl (object value)
		{
			if (value == null)
				throw new ArgumentException ();

			for (int i=0; i < Count; i++) {
				if (this [i].Equals (value))
					return i;
			}
			return -1;			
		}

		internal override bool IsReadOnlyImpl ()
		{
			return readOnly;
		}
	}
}
