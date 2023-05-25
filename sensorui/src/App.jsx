import './App.css';
import 'bootstrap/dist/css/bootstrap.min.css';
import { Container, Row, Col } from 'react-bootstrap';
import useFetchInterval from './useFetchInterval';

function App() {

  const { data, loading, error } = useFetchInterval('https://www.so718.sohost.pl/sensor/getSensor.php', 15000);

  return (
    <>
      <Container fluid className="h-100 d-flex flex-column">
        <Row className="flex-grow-1">
          <Col className="d-flex align-items-center justify-content-center p-3">
            <h1 className='data'>{data && <>
              {data.temperature} °C
            </>}</h1>
          </Col>
        </Row>
        <Row className="flex-grow-1">
          <Col className="d-flex align-items-center justify-content-center p-3">
            <h1 className='data'>{data && <>
              {data.humidity} %
            </>}</h1>
          </Col>
        </Row>
      </Container>
    </>
  )
}

export default App
