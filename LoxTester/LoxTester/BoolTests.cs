// Generated using LoxTestGenerator

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace LoxTester
{
	[TestClass]
	public class BoolTests
	{
		[TestMethod]
		public void EqualityTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\bool\\equality.lox");
			Assert.AreEqual(18, output.Length);
			Assert.AreEqual("true", output[0]);
			Assert.AreEqual("false", output[1]);
			Assert.AreEqual("false", output[2]);
			Assert.AreEqual("true", output[3]);
			Assert.AreEqual("false", output[4]);
			Assert.AreEqual("false", output[5]);
			Assert.AreEqual("false", output[6]);
			Assert.AreEqual("false", output[7]);
			Assert.AreEqual("false", output[8]);
			Assert.AreEqual("false", output[9]);
			Assert.AreEqual("true", output[10]);
			Assert.AreEqual("true", output[11]);
			Assert.AreEqual("false", output[12]);
			Assert.AreEqual("true", output[13]);
			Assert.AreEqual("true", output[14]);
			Assert.AreEqual("true", output[15]);
			Assert.AreEqual("true", output[16]);
			Assert.AreEqual("true", output[17]);
		}

		[TestMethod]
		public void NotTest()
		{
			string[] output = Tools.RunFile("C:\\Users\\Kai\\source\\repos\\LoxMin\\LoxTester\\\\LoxTester\\Tests\\bool\\not.lox");
			Assert.AreEqual(3, output.Length);
			Assert.AreEqual("false", output[0]);
			Assert.AreEqual("true", output[1]);
			Assert.AreEqual("true", output[2]);
		}

	}
}
