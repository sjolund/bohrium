using System;
using System.Collections.Generic;
using NumCIL.Generic;

namespace NumCIL
{
	/// <summary>
	/// Abstract syntax tree implementation for code analysis
	/// </summary>
	public class AST
	{
		/// <summary>
		/// Node representation for an operation
		/// </summary>
		public class Node
		{
			public readonly Node Target;
			public readonly Node Left;
            public readonly Node Right;
            public readonly Node[] Others;

			public Node(Node target, Node left, Node right, params Node[] others)
			{
				this.Target = target;
				this.Left = left;
                this.Right = right;
                this.Others = others;
            }
		}

		public static List<Node> BuildAST(IEnumerable<object> operations)
		{
            

		}
	}
}

