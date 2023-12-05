// Generated using LoxTestGenerator

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace LoxTester
{
	[TestClass]
	public class OperatorTests
	{
		[TestMethod]
		public void AddTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("579", output[0]);
			Assert.AreEqual("string", output[1]);
		}

		[TestMethod]
		public void AddBoolNilTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_bool_nil.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void AddBoolNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_bool_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void AddBoolStringTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_bool_string.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void AddNilNilTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_nil_nil.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void AddNumNilTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_num_nil.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void AddStringNilTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\add_string_nil.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be two numbers or two strings.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void ComparisonTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\comparison.lox");
			Assert.AreEqual(20, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("false", output[1]);
			Assert.AreEqual("false", output[2]);
			Assert.AreEqual("true", output[3]);
			Assert.AreEqual("true", output[4]);
			Assert.AreEqual("false", output[5]);
			Assert.AreEqual("false", output[6]);
			Assert.AreEqual("false", output[7]);
			Assert.AreEqual("true", output[8]);
			Assert.AreEqual("false", output[9]);
			Assert.AreEqual("true", output[10]);
			Assert.AreEqual("true", output[11]);
			Assert.AreEqual("false", output[12]);
			Assert.AreEqual("false", output[13]);
			Assert.AreEqual("false", output[14]);
			Assert.AreEqual("false", output[15]);
			Assert.AreEqual("true", output[16]);
			Assert.AreEqual("true", output[17]);
			Assert.AreEqual("true", output[18]);
			Assert.AreEqual("true", output[19]);
		}

		[TestMethod]
		public void DivideTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\divide.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("4", output[0]);
			Assert.AreEqual("1", output[1]);
		}

		[TestMethod]
		public void DivideNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\divide_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void DivideNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\divide_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void EqualsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\equals.lox");
			Assert.AreEqual(10, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("true", output[1]);
			Assert.AreEqual("false", output[2]);
			Assert.AreEqual("true", output[3]);
			Assert.AreEqual("false", output[4]);
			Assert.AreEqual("true", output[5]);
			Assert.AreEqual("false", output[6]);
			Assert.AreEqual("false", output[7]);
			Assert.AreEqual("false", output[8]);
			Assert.AreEqual("false", output[9]);
		}

		[TestMethod]
		public void EqualsClassTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\equals_class.lox");
			Assert.AreEqual(8, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("false", output[1]);
			Assert.AreEqual("false", output[2]);
			Assert.AreEqual("true", output[3]);
			Assert.AreEqual("false", output[4]);
			Assert.AreEqual("false", output[5]);
			Assert.AreEqual("false", output[6]);
			Assert.AreEqual("false", output[7]);
		}

		[TestMethod]
		public void EqualsMethodTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\equals_method.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("false", output[1]);
		}

		[TestMethod]
		public void GreaterNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\greater_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void GreaterNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\greater_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void GreaterOrEqualNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\greater_or_equal_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void GreaterOrEqualNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\greater_or_equal_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void LessNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\less_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void LessNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\less_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void LessOrEqualNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\less_or_equal_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void LessOrEqualNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\less_or_equal_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void MultiplyTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\multiply.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("15", output[0]);
			Assert.AreEqual("3.702", output[1]);
		}

		[TestMethod]
		public void MultiplyNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\multiply_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void MultiplyNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\multiply_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void NegateTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\negate.lox");
			Assert.AreEqual(3, output.Length);
			Assert.AreEqual("-3", output[0]);
			Assert.AreEqual("3", output[1]);
			Assert.AreEqual("-3", output[2]);
		}

		[TestMethod]
		public void NegateNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\negate_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operand must be a number.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void NotTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\not.lox");
			Assert.AreEqual(8, output.Length);
			Assert.AreEqual("false", output[0]);
			Assert.AreEqual("true", output[1]);
			Assert.AreEqual("true", output[2]);
			Assert.AreEqual("false", output[3]);
			Assert.AreEqual("false", output[4]);
			Assert.AreEqual("true", output[5]);
			Assert.AreEqual("false", output[6]);
			Assert.AreEqual("false", output[7]);
		}

		[TestMethod]
		public void NotClassTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\not_class.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("false", output[0]);
			Assert.AreEqual("false", output[1]);
		}

		[TestMethod]
		public void NotEqualsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\not_equals.lox");
			Assert.AreEqual(10, output.Length);
			Assert.AreEqual("false", output[0]);
			Assert.AreEqual("false", output[1]);
			Assert.AreEqual("true", output[2]);
			Assert.AreEqual("false", output[3]);
			Assert.AreEqual("true", output[4]);
			Assert.AreEqual("false", output[5]);
			Assert.AreEqual("true", output[6]);
			Assert.AreEqual("true", output[7]);
			Assert.AreEqual("true", output[8]);
			Assert.AreEqual("true", output[9]);
		}

		[TestMethod]
		public void SubtractTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\subtract.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("1", output[0]);
			Assert.AreEqual("0", output[1]);
		}

		[TestMethod]
		public void SubtractNonnumNumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\subtract_nonnum_num.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

		[TestMethod]
		public void SubtractNumNonnumTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\operator\\subtract_num_nonnum.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Operands must be numbers.", output[0]);
			Assert.AreEqual("[line 1] in script", output[1]);
		}

	}
}
