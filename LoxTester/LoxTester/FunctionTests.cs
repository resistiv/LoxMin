// Generated using LoxTestGenerator

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace LoxTester
{
	[TestClass]
	public class FunctionTests
	{
		[TestMethod]
		public void BodyMustBeBlockTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\body_must_be_block.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("[line 3] Error at '123': Expect '{' before function body.", output[0]);
			Assert.AreEqual("[line 4] Error at end: Expect '}' after block.", output[1]);
		}

		[TestMethod]
		public void EmptyBodyTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\empty_body.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("nil", output[0]);
		}

		[TestMethod]
		public void ExtraArgumentsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\extra_arguments.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Expected 2 arguments but got 4.", output[0]);
			Assert.AreEqual("[line 6] in script", output[1]);
		}

		[TestMethod]
		public void LocalMutualRecursionTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\local_mutual_recursion.lox");
			Assert.AreEqual(3, output.Length);
			Assert.AreEqual("Undefined variable 'isOdd'.", output[0]);
			Assert.AreEqual("[line 4] in isEven()", output[1]);
			Assert.AreEqual("[line 12] in script", output[2]);
		}

		[TestMethod]
		public void LocalRecursionTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\local_recursion.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("21", output[0]);
		}

		[TestMethod]
		public void MissingArgumentsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\missing_arguments.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("Expected 2 arguments but got 1.", output[0]);
			Assert.AreEqual("[line 3] in script", output[1]);
		}

		[TestMethod]
		public void MissingCommaInParametersTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\missing_comma_in_parameters.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("[line 3] Error at 'c': Expect ')' after parameters.", output[0]);
			Assert.AreEqual("[line 4] Error at end: Expect '}' after block.", output[1]);
		}

		[TestMethod]
		public void MutualRecursionTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\mutual_recursion.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("true", output[1]);
		}

		[TestMethod]
		public void NestedCallWithArgumentsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\nested_call_with_arguments.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("hello world", output[0]);
		}

		[TestMethod]
		public void ParametersTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\parameters.lox");
			Assert.AreEqual(9, output.Length);
			Assert.AreEqual("0", output[0]);
			Assert.AreEqual("1", output[1]);
			Assert.AreEqual("3", output[2]);
			Assert.AreEqual("6", output[3]);
			Assert.AreEqual("10", output[4]);
			Assert.AreEqual("15", output[5]);
			Assert.AreEqual("21", output[6]);
			Assert.AreEqual("28", output[7]);
			Assert.AreEqual("36", output[8]);
		}

		[TestMethod]
		public void PrintTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\print.lox");
			Assert.AreEqual(2, output.Length);
			Assert.AreEqual("<fn foo>", output[0]);
			Assert.AreEqual("<native fn>", output[1]);
		}

		[TestMethod]
		public void RecursionTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\recursion.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("21", output[0]);
		}

		[TestMethod]
		public void TooManyArgumentsTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\too_many_arguments.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("[line 260] Error at 'a': Can't have more than 255 arguments.", output[0]);
		}

		[TestMethod]
		public void TooManyParametersTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\function\\too_many_parameters.lox");
			Assert.AreEqual(1, output.Length);
			Assert.AreEqual("[line 257] Error at 'a': Can't have more than 255 parameters.", output[0]);
		}

	}
}
